#include <QDebug>
#include <QQmlContext>
#include <QQmlProperty>
#include <QDateTime>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>

#include "utils/fileutils.h"
#include "ui.h"
#include "section/deviceinfo.h"
#include "section/manual.h"
#include "section/provision.h"

UI::MainUI::MainUI(const QGuiApplication& app, LogObject* logData)
    : QObject(),
      mApp(app),
      mSection(nullptr)
{
    mDevicesModel = new UI::ConnectedDevicesModel();
    QObject::connect(this, &UI::MainUI::deviceChanged, mDevicesModel, &UI::ConnectedDevicesModel::deviceChanged);
    QObject::connect(this, &UI::MainUI::deviceUpdate, mDevicesModel, &UI::ConnectedDevicesModel::deviceUpdate);

    QDateTime buildTime;
    buildTime.setTime_t(QString(PROG_BUILDTIME).toULongLong());

    mEngine = new QQmlApplicationEngine();
    mEngine->rootContext()->setContextProperty("logText", logData);
    mEngine->rootContext()->setContextProperty("programVersion", QString(PROG_VERSION));
    mEngine->rootContext()->setContextProperty("programBuildTime", QVariant::fromValue(buildTime));
    mEngine->rootContext()->setContextProperty("qtVersion", QString(QT_VERSION_STR));
    mEngine->rootContext()->setContextProperty("devicesModel", mDevicesModel);
    mEngine->rootContext()->setContextProperty("userTokenSet", QSettings().contains("user/token"));

    QObject::connect(mEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    mEngine->load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

    mRescheduleTimer = new QTimer();
    mRescheduleTimer->setSingleShot(true);
    mRescheduleTimer->setInterval(5000);
    mRescheduleTimer->setTimerType(Qt::TimerType::VeryCoarseTimer);
    connect(mRescheduleTimer, &QTimer::timeout, this, &UI::MainUI::devicesChanged);

    /* Set up signals */
    QObject* rootObject = mEngine->rootObjects().first();

    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");
    QObject::connect(pageLoader, SIGNAL(viewChanged()), this, SLOT(viewChanged()));

    QObject* connectedDevicesList = rootObject->findChild<QObject*>("connectedDevicesList");
    QObject::connect(connectedDevicesList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentDeviceChanged(int)));
    QObject::connect(connectedDevicesList, SIGNAL(refresh()), this, SLOT(devicesChanged()));

    QObject::connect(rootObject->findChild<QObject*>("loginButton"), SIGNAL(clicked()), this, SLOT(login()));
    QObject::connect(rootObject->findChild<QObject*>("fileMenuLogout"), SIGNAL(triggered()), this, SLOT(logout()));

    QObject::connect(rootObject->findChild<QObject*>("provisionFailedContainer"), SIGNAL(closed()), this, SLOT(provisionFailedClose()));

    QThread* thread = new QThread;

    mWorker = new SerialDeviceWorker(logData);
    mWorker->moveToThread(thread);

    connect(mWorker, &SerialDeviceWorker::devicesListChanged, this, &MainUI::devicesListChanged);
    connect(mWorker, &SerialDeviceWorker::deviceAdd, this, &MainUI::deviceAdd);
    connect(mWorker, &SerialDeviceWorker::deviceAddReschedule, this, &MainUI::deviceAddReschedule);
    connect(mWorker, &SerialDeviceWorker::deviceClose, this, &MainUI::deviceClose);
    connect(mWorker, &SerialDeviceWorker::loginStatus, this, &MainUI::loginStatusChanged);
    connect(mWorker, &SerialDeviceWorker::statusChange, this, &MainUI::setStatus);
    connect(mWorker, &SerialDeviceWorker::provisionProgressChanged, mDevicesModel, &ConnectedDevicesModel::setProgress);
    connect(mWorker, &SerialDeviceWorker::updateCheck, this, &MainUI::versionUpdateCheck);
    connect(mWorker, &SerialDeviceWorker::updateAvailable, this, &MainUI::versionUpdateAvailable);

    connect(thread, SIGNAL(started()), mWorker, SLOT(process()));
    connect(mWorker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(mWorker, SIGNAL(finished()), mWorker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), mWorker, SLOT(deleteLater()));

    viewChanged();
    currentDeviceChanged(connectedDevicesList->property("currentIndex").toInt());

    thread->start();

    QSettings settings;
    if(settings.contains("user/token")) {
        mWorker->addLoginCheck(settings.value("user/token").toString());
    }
}

UI::MainUI::~MainUI() {
    if(mSection != nullptr) {
        delete mSection;
    }

    mWorker->stop();

    delete mEngine;
    delete mDevicesModel;
    delete mRescheduleTimer;
}

void UI::MainUI::devicesListChanged(bool success) {
    qDebug()<<"devicesListChanged"<<success;
    emit loggedIn();
}

void UI::MainUI::devicesChanged() {
    mRescheduleTimer->stop();

    qDebug()<<"handleDeviceAdded availablePorts"<<QSerialPortInfo::availablePorts().size();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString port = info.portName();

        bool shouldAdd = true;
        foreach(Serial::SerialDevice* d, mDevices) {
            if(*d == port) {
                shouldAdd = false;
                break;
            }
        }
        if(shouldAdd) {
            foreach(Serial::SerialDevice* d, mPendingDevices) {
                if(*d == port) {
                    shouldAdd = false;
                    break;
                }
            }
        }

        if(shouldAdd) {
            mWorker->addDeviceCheck(info);
        } else {
            qDebug()<<"Not checking port"<<port;
        }
    }

    if(mRescheduledDevices.size() > 0) {
        mRescheduleTimer->start();
    }
}

void UI::MainUI::deviceAdd(Serial::SerialDevice* device) {
    qDebug()<<"deviceAdd"<<device->port();

    mRescheduleTimer->stop();
    if(mRescheduledDevices.contains(device->port())) {
        mRescheduledDevices.remove(device->port());
    }
    if(mRescheduledDevices.size() > 0) {
        mRescheduleTimer->start();
    }

    // Do we already have this device?
    foreach(Serial::SerialDevice* d, mDevices) {
        if(*d == *device) {
            delete device;
            return;
        }
    }
    foreach(Serial::SerialDevice* d, mPendingDevices) {
        if(*d == *device) {
            delete device;
            return;
        }
    }

    // Check if this device is a child to another device.
    for(int i = 0; i < mDevices.size(); i++) {
        Serial::SerialDevice* d = mDevices.at(i);
        qDebug()<<"deviceAdd"<<d->vidStr()<<device->vidStr()<<d->pidStr()<<device->pidStr()<<d->meidStr()<<device->meidStr();

        if(device->isSameDevice(d)) {
            // If the new device has a make, mdn or a min, use it as the parent. Otherwise we add this one as a child.
            if(device->make().length() > d->make().length() || (device->mdn().size() > 0 && d->mdn().size() == 0) || (device->min() != 0 && d->min() == 0)) {
                device->addChild(d);
                mDevices.replace(i, device);
                d = device;
            } else {
                d->addChild(device);
            }

            emit deviceUpdate(d);
            viewUpdate();
            return;
        }
    }

    foreach(Serial::SerialDevice* d, mPendingDevices) {
        if(device->isSameDevice(d)) {
            device->addChild(d);
            mPendingDevices.removeOne(d);
            break;
        }
    }

#ifndef TESTING_MODE
    if(device->make().size() == 0) {
        mPendingDevices.append(device);
        return;
    }
#endif

    qInfo()<<"Adding port"<<device->port();

    mDevices.append(device);

    emit deviceChanged(device, true);

    viewUpdate();
}

void UI::MainUI::deviceAddReschedule(QString port) {
    qWarning()<<"Rescheduling check for"<<port;

    mRescheduleTimer->stop();
    if(mRescheduledDevices.contains(port)) {
        mRescheduledDevices.remove(port);
    } else {
        mRescheduledDevices.insert(port);
    }

    if(mRescheduledDevices.size() > 0) {
        mRescheduleTimer->start();
    }
}

void UI::MainUI::deviceClose(Serial::SerialDevice *device) {
    emit deviceUpdate(device);

    viewUpdate();
}

void UI::MainUI::deviceRemove(const QString& port) {
    for(int i = 0; i < mDevices.size(); i++) {
        if(*mDevices.at(i) == port) {
            Serial::SerialDevice* device = mDevices[i];
            if(device->isProvisioned()) {
                mWorker->addDeviceClose(device);
            } else {
                emit deviceChanged(device, false);
                mDevices.removeAt(i);

                mWorker->addDeviceRemove(device);
            }

            break;
        }
    }
    for(int i = 0; i < mPendingDevices.size(); i++) {
        if(*mPendingDevices.at(i) == port) {
            Serial::SerialDevice* device = mPendingDevices[i];
            mPendingDevices.removeAt(i);

            mWorker->addDeviceRemove(device);

            break;
        }
    }


    viewUpdate();
}

void UI::MainUI::currentDeviceChanged(int index) {
    if(mSection != nullptr) {
        mSection->beforeDeviceChanged();
    }

    mCurrentIndex = index;
    viewUpdate();
}

void UI::MainUI::setStatus(const QString &status) {
    QObject* rootObject = mEngine->rootObjects().first();
    QObject* statusBarText = rootObject->findChild<QObject*>("statusBarText");
    statusBarText->setProperty("text", status);
}

void UI::MainUI::viewChanged() {
    QObject* rootObject = mEngine->rootObjects().first();
    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");

    QString view = pageLoader->property("currentView").toString();
    qDebug()<<"viewChanged"<<view;

    if(mSection != nullptr) {
        delete mSection;
    }
    mSection = nullptr;

    if(view == "manual") {
        mSection = new UI::Section::Manual(this);
    } else if(view == "provision") {
        mSection = new UI::Section::Provision(this);
    } else if(view == "deviceinfo") {
        mSection = new UI::Section::DeviceInfo(this);
    }

    if(mSection == nullptr) {
        qWarning()<<"Could not find a handler for section"<<view;
        return;
    }

    viewUpdate();
}

void UI::MainUI::viewUpdate() {
    qDebug() << "viewUpdate";

    if(mSection == nullptr) {
        return;
    }

    mSection->update();
}

void UI::MainUI::login() {
    QObject* rootObject = mEngine->rootObjects().first();
    QString username = rootObject->findChild<QObject*>("usernameText")->property("text").toString();
    QString password = rootObject->findChild<QObject*>("passwordText")->property("text").toString();

    rootObject->findChild<QObject*>("loggingInOverlay")->setProperty("opacity", 1.0f);

    mWorker->addLogin(username, password);
}

void UI::MainUI::logout() {
    QSettings settings;
    settings.remove("user/token");

    updateLoginStatus(false);

    emit loggedOut();

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        deviceRemove(info.portName());
    }
}

void UI::MainUI::loginStatusChanged(bool success, const QString& token) {
    qDebug()<<"loginStatusChanged"<<success;
    if(!success) {
        updateLoginStatus(false);

        QObject* rootObject = mEngine->rootObjects().first();
        QMetaObject::invokeMethod(rootObject->findChild<QObject*>("loginFailedContainer"), "setError", Q_ARG(QVariant, "Please check your username or password."));

        return;
    }

    QSettings settings;
    settings.setValue("user/token", token);

    qInfo()<<"Logged in successfully!";

    mWorker->addDevicesListUpdate();

    updateLoginStatus(true);
}

void UI::MainUI::updateLoginStatus(bool loggedIn) {
    qDebug()<<"updateLoginStatus"<<loggedIn;

    QObject* rootObject = mEngine->rootObjects().first();

    rootObject->findChild<QObject*>("fileMenuLogout")->setProperty("visible", loggedIn);
    rootObject->findChild<QObject*>("editMenu")->setProperty("visible", loggedIn);
    // Show advanced menu item if we are using a testing build
#ifdef TESTING_MODE
    rootObject->findChild<QObject*>("advancedMenu")->setProperty("visible", loggedIn);
#endif

    if(loggedIn) {
        rootObject->findChild<QObject*>("usernameText")->setProperty("text", "");
        rootObject->findChild<QObject*>("passwordText")->setProperty("text", "");

        rootObject->findChild<QObject*>("loggingInOverlay")->setProperty("opacity", 0.0f);
    } else {
        QMetaObject::invokeMethod(rootObject->findChild<QObject*>("loginOverlay"), "show");
    }
}

void UI::MainUI::versionUpdateCheck() {
    QObject* rootObject = mEngine->rootObjects().first();
    QMetaObject::invokeMethod(rootObject->findChild<QObject*>("loggingInView"), "setCheckForUpdates", Q_ARG(QVariant, true));
}

void UI::MainUI::versionUpdateAvailable(const QString& updaterDir, const QString& token, const QString &id, const QString &version, const QDateTime &updateTime) {
    qDebug()<<"New Update is available"<<version;

    QSettings settings;
    settings.setValue("user/token", token);
    settings.sync();

#ifdef Q_OS_WIN
    QString updaterPath(updaterDir + "/Updater.exe");
#else
    QString updaterPath(updaterDir + "/Updater");
#endif

    QStringList argumentsList;
    argumentsList << id << version << QCoreApplication::applicationDirPath();

    Utils::FileUtils::execute(updaterPath, argumentsList, updaterDir);

    mApp.quit();
}

void UI::MainUI::provisionFailedClose() {
    Serial::SerialDevice* device = mDevices.at(currentIndex());
    if(device == nullptr) {
        return;
    }

    device->setProvisioning(false);
    mDevicesModel->setProgress(device, Serial::SerialProvisionStatusIdle, 0, Serial::SerialProvisionErrorNone);

    viewUpdate();
}

UI::UISection::UISection(UI::MainUI* ui)
    : QObject(),
      mUI(ui)
{

}

void UI::UISection::beforeDeviceChanged() {

}

void UI::UISection::startUpdate() {
    QObject* rootObject = this->rootObject();

    QObject* currentDeviceLabel = rootObject->findChild<QObject*>("currentDeviceNameLabel");
    if(devices().size() == 0) {
        QMetaObject::invokeMethod(currentDeviceLabel, "reset");
    } else {
        currentDeviceLabel->setProperty("text", currentDevice()->name());
    }

    if(devices().size() == 0) {
        QObject* bottomTab = rootObject->findChild<QObject*>("mainPageBottomTabArrow");
        bottomTab->setProperty("hidden", true);
        bottomTab->setProperty("enabled", false);

        rootObject->findChild<QObject*>("noDeviceOverlay")->setProperty("opacity", 1.0f);
    } else {
        Serial::SerialDevice* currentDevice = UISection::currentDevice();
        if(currentDevice != nullptr) {
            rootObject->findChild<QObject*>("currentDeviceMake")->setProperty("value", currentDevice->makeStr());
            rootObject->findChild<QObject*>("currentDeviceModel")->setProperty("value", currentDevice->modelStr());
            rootObject->findChild<QObject*>("currentDeviceMDN")->setProperty("value", currentDevice->mdnStr());
            rootObject->findChild<QObject*>("currentDeviceMIN")->setProperty("value", currentDevice->minStr());
            rootObject->findChild<QObject*>("currentDeviceESN")->setProperty("value", currentDevice->esnStr());
            rootObject->findChild<QObject*>("currentDeviceMEID")->setProperty("value", currentDevice->meidStr());
            rootObject->findChild<QObject*>("currentDeviceIMEI")->setProperty("value", currentDevice->imeiStr());
        }
    }
}

void UI::UISection::endUpdate() {
    QObject* rootObject = this->rootObject();
    if(devices().size() > 0) {
        QObject* bottomTab = rootObject->findChild<QObject*>("mainPageBottomTabArrow");
        bottomTab->setProperty("enabled", true);
        bottomTab->setProperty("hidden", false);

        rootObject->findChild<QObject*>("noDeviceOverlay")->setProperty("opacity", 0.0f);
    } else {
        rootObject->findChild<QObject*>("currentDeviceMake")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceModel")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceMDN")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceMIN")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceESN")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceMEID")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceIMEI")->setProperty("value", "");
    }
}

Serial::SerialDevice* UI::UISection::currentDevice() {
    // The index defaults to -1, so we check the first item if it's the default.
    int currentIndex = ui()->currentIndex();
    if(currentIndex < 0) {
        currentIndex = 0;
    }
    if(currentIndex >= devices().size()) {
        return nullptr;
    }

    return devices().at(currentIndex);
}
