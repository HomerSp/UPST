#include <QDebug>
#include <QQmlContext>
#include <QQmlProperty>
#include <QDateTime>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QThread>

#include "utils/fileutils.h"
#include "web/webutils.h"
#include "ui.h"
#include "section/deviceinfo.h"
#include "section/manual.h"
#include "section/provision.h"

UI::MainUI::MainUI(const QGuiApplication& app, Log::LogHandler* logHandler, bool updateFailed)
    : QObject(),
      mApp(app),
      mSection(nullptr),
      mLogHandler(logHandler)
{
    mDevicesModel = new UI::ConnectedDevicesModel(this);
    QObject::connect(this, &UI::MainUI::deviceChanged, mDevicesModel, &UI::ConnectedDevicesModel::deviceChanged);
    QObject::connect(this, &UI::MainUI::deviceUpdate, mDevicesModel, &UI::ConnectedDevicesModel::deviceUpdate);

    mDeviceGuideListModel = new UI::Guide::DeviceGuideListModel(this);
    mUserGuideHelper = new UI::Guide::UserGuideHelper(this);

    QDateTime buildTime;
    buildTime.setTime_t(QString(PROG_BUILDTIME).toULongLong());

    mEngine = new QQmlApplicationEngine();
    mEngine->rootContext()->setContextProperty("logText", logHandler);
    mEngine->rootContext()->setContextProperty("programVersion", QString(PROG_VERSION));
    mEngine->rootContext()->setContextProperty("programBuildTime", QVariant::fromValue(buildTime));
#ifdef TESTING_MODE
    mEngine->rootContext()->setContextProperty("programTestingMode", QVariant::fromValue(true));
#else
    mEngine->rootContext()->setContextProperty("programTestingMode", QVariant::fromValue(false));
#endif
    mEngine->rootContext()->setContextProperty("updateFailed", updateFailed);
    mEngine->rootContext()->setContextProperty("qtVersion", QString(QT_VERSION_STR));
    mEngine->rootContext()->setContextProperty("devicesModel", mDevicesModel);
    mEngine->rootContext()->setContextProperty("userTokenSet", QSettings().contains("user/token"));
    mEngine->rootContext()->setContextProperty("userDisplayName", "");
    mEngine->rootContext()->setContextProperty("userGuideHelper", mUserGuideHelper);
    mEngine->rootContext()->setContextProperty("deviceGuideListModel", mDeviceGuideListModel);

    QObject::connect(mEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    mEngine->load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

    /* Set up signals */
    QObject* rootObject = mEngine->rootObjects().first();

    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");
    QObject::connect(pageLoader, SIGNAL(viewChanged()), this, SLOT(viewChanged()));

    QObject* connectedDevicesList = rootObject->findChild<QObject*>("connectedDevicesList");
    QObject::connect(connectedDevicesList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentDeviceChanged(int)));
    QObject::connect(connectedDevicesList, SIGNAL(refresh()), this, SLOT(doRefresh()));

    QObject::connect(rootObject->findChild<QObject*>("loginButton"), SIGNAL(clicked()), this, SLOT(login()));

    QObject::connect(rootObject->findChild<QObject*>("editMenuProvisionAll"), SIGNAL(triggered()), this, SLOT(provisionAll()));
    QObject::connect(rootObject->findChild<QObject*>("importDevicesDialog"), SIGNAL(importDevices(const QUrl&)), this, SLOT(importDevices(const QUrl&)));
    QObject::connect(rootObject->findChild<QObject*>("userMenuLogout"), SIGNAL(triggered()), this, SLOT(logout()));

    QObject::connect(rootObject->findChild<QObject*>("provisionFailedContainer"), SIGNAL(closed()), this, SLOT(provisionFailedClose()));
    QObject::connect(rootObject->findChild<QObject*>("provisionSuccessContainer"), SIGNAL(closed()), this, SLOT(provisionFailedClose()));
    QObject::connect(rootObject->findChild<QObject*>("provisionRebootingContainer"), SIGNAL(closed()), this, SLOT(provisionFailedClose()));
    QObject::connect(rootObject->findChild<QObject*>("provisionWrongSPCContainer"), SIGNAL(closed()), this, SLOT(provisionFailedClose()));

    QObject::connect(rootObject->findChild<QObject*>("updateFailedContinueButton"), SIGNAL(clicked()), this, SLOT(updateFailedContinue()));

    mWorker = new UI::Worker::UIWorker(updateFailed);
    connect(mWorker, &UI::Worker::UIWorker::devicesListChanged, this, &MainUI::devicesListChanged);
    connect(mWorker, &UI::Worker::UIWorker::loginStatus, this, &MainUI::loginStatusChanged);
    connect(mWorker, &UI::Worker::UIWorker::updateCheck, this, &MainUI::versionUpdateCheck);
    connect(mWorker, &UI::Worker::UIWorker::updateAvailable, this, &MainUI::versionUpdateAvailable);
    connect(mWorker, &UI::Worker::UIWorker::statusChange, this, &MainUI::setStatus);

    mDeviceWorker = new UI::Worker::SerialDeviceWorker();
    connect(mDeviceWorker, &UI::Worker::SerialDeviceWorker::deviceAdd, this, &MainUI::deviceAddChecked);
    connect(mDeviceWorker, &UI::Worker::SerialDeviceWorker::deviceAddReschedule, this, &MainUI::deviceAddReschedule);
    connect(mDeviceWorker, &UI::Worker::SerialDeviceWorker::deviceClose, this, &MainUI::deviceClose);
    connect(mDeviceWorker, &UI::Worker::SerialDeviceWorker::deviceUpdated, this, &MainUI::deviceUpdated);
    connect(mDeviceWorker, &UI::Worker::SerialDeviceWorker::deviceBatchLoaded, this, &MainUI::deviceBatchLoaded);
    connect(mDeviceWorker, &UI::Worker::SerialDeviceWorker::provisionProgressChanged, this, &MainUI::provisionProgressChanged);
    connect(mDeviceWorker, &UI::Worker::SerialDeviceWorker::statusChange, this, &MainUI::setStatus);

    viewChanged();
    currentDeviceChanged(connectedDevicesList->property("currentIndex").toInt());

    mWorker->start();
    mDeviceWorker->start();

    QSettings settings;
    if(settings.contains("user/token")) {
        mWorker->addLoginCheck(settings.value("user/token").toString());
    }
}

UI::MainUI::~MainUI() {
    while(mDevices.size() > 0) {
        Serial::SerialDevice* device = mDevices.first();

        emit deviceChanged(device, false);
        mDevices.removeAt(0);

        mDeviceWorker->addDeviceRemove(device);
    }

    if(mSection != nullptr) {
        delete mSection;
    }

    mDeviceWorker->stop();
    mWorker->stop();

    delete mEngine;
    delete mDevicesModel;
}

void UI::MainUI::devicesListChanged(bool success, Serial::SerialDeviceConfig* config) {
    qDebug()<<"devicesListChanged main"<<success;
    if(success) {
        mDeviceGuideListModel->setDeviceConfig(config);
        mDeviceWorker->setDeviceConfig(config);

        mEngine->rootObjects().first()->findChild<QObject*>("helpMenuDeviceGuide")->setProperty("visible", true);
    }

    emit loggedIn();
}

void UI::MainUI::deviceAdd(const QString& port) {
    qInfo()<<"deviceAdd"<<port;

    bool shouldAdd = true;
    foreach(Serial::SerialDevice* d, mDevices) {
        if(*d == port && !d->isProvisioning()) {
            shouldAdd = false;
            break;
        }
    }

    if(shouldAdd) {
        mDeviceWorker->addDeviceCheck(QSerialPortInfo(port));
    } else {
        qDebug()<<"Not checking port"<<port;
    }
}

void UI::MainUI::deviceAddChecked(Serial::SerialDevice* device) {
    qDebug()<<"deviceAddChecked"<<device->port();

    mDeviceRechecks.remove(device->port());

    // Is this a device that's been provisioned that has reappared?
    for(int i = 0; i < mDevices.size(); i++) {
        Serial::SerialDevice* d = mDevices.at(i);
        if(d->isSameDevice(device) && d->isProvisioning() && !d->isAvailable()) {
            qDebug()<<"Devices are identical, replacing old"<<d->port()<<"with new"<<device->port();
            d->updateFrom(device);

            emit deviceUpdate(d);

            mDeviceWorker->addDeviceRemove(device);

            if(d->isAvailable() && (!d->flagMultiPort() || (d->flagMultiPort() && d->childrenAvailable())) && d->rtreSet()) {
                d->setProvisioning(false);

                if(d->canProvision()) {
                    d->setProvisioning(true);
                    provisionProgressChanged(d, Serial::SerialProvisionStatusQueue, 0, Serial::SerialProvisionErrorNone);
                    viewUpdate();

                    mDeviceWorker->addDeviceProvision(d);
                }
            }

            viewUpdate();

            return;
        }
    }

    // Do we already have this device?
    foreach(Serial::SerialDevice* d, mDevices) {
        if(*d == *device) {
            // Don't delete a device directly - call deleteLater instead, this causes problems as the device was created in another thread.
            qInfo()<<"Deleting already existing device"<<device->portStr();
            device->deleteLater();
            return;
        }
    }

    // Check if this device is a child to another device.
    for(int i = 0; i < mDevices.size(); i++) {
        Serial::SerialDevice* d = mDevices.at(i);
        qDebug()<<"deviceAddChecked"<<d->vidStr()<<device->vidStr()<<d->pidStr()<<device->pidStr()<<d->meidStr()<<device->meidStr();

        if(device->isSameDevice(d)) {
            // If the new device has a type, use it as the parent. Otherwise we add this one as a child.
            if(device->type() != Serial::SerialDeviceTypeUnknown && d->type() == Serial::SerialDeviceTypeUnknown) {
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

    qInfo()<<"Adding port"<<device->port();

    mDevices.append(device);

    emit deviceChanged(device, true);

    viewUpdate();
}

void UI::MainUI::deviceAddReschedule(QString port) {
    if(mDeviceRechecks.contains(port) && mDeviceRechecks.value(port) >= 3) {
        qCritical()<<"Failed to check device"<<port<<"not checking again.";
        return;
    }

    qWarning()<<"Rescheduling check for"<<port;

    int recheckCount = (mDeviceRechecks.contains(port))?mDeviceRechecks.value(port)+1:0;
    mDeviceRechecks.insert(port, recheckCount);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &UI::MainUI::deviceRescheduleTimeout);
    timer->setInterval(2000);
    timer->setSingleShot(true);
    timer->setProperty("port", port);
    timer->start();
}

void UI::MainUI::deviceClose(Serial::SerialDevice *device) {
    if(device != nullptr) {
        qDebug()<<"deviceClose"<<device->port();

        for(int i = 0; i < mDevices.size(); i++) {
            if(mDevices.at(i) == device) {
                emit deviceUpdate(device);
                break;
            }
        }
    }

    viewUpdate();
}

void UI::MainUI::deviceRemove(const QString& port) {
    qInfo()<<"deviceRemove"<<port;

    mDeviceRechecks.remove(port);

    for(int i = 0; i < mDevices.size(); i++) {
        if(*mDevices.at(i) == port) {
            Serial::SerialDevice* device = mDevices.at(i);
            if(device->isProvisioning()) {
                mDeviceWorker->addDeviceClose(device);
            } else {
                emit deviceChanged(device, false);
                mDevices.removeAt(i);

                mDeviceWorker->addDeviceRemove(device);
            }

            break;
        }
    }

    viewUpdate();
}

void UI::MainUI::deviceRescheduleTimeout() {
    QTimer* timer = static_cast<QTimer*>(sender());
    if(timer == nullptr) {
        return;
    }

    QString port = timer->property("port").toString();
    if(mDeviceRechecks.contains(port) && !QSerialPortInfo(port).isNull()) {
        deviceAdd(port);
    } else {
        mDeviceRechecks.remove(port);
    }

    timer->deleteLater();
}

void UI::MainUI::deviceUpdated(Serial::SerialDevice *device) {
    emit deviceUpdate(device);
    viewUpdate();
}

void UI::MainUI::deviceBatchLoaded() {
    foreach(Serial::SerialDevice* d, mDevices) {
        mDeviceWorker->addDeviceUpdate(d);
    }
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
        mSection->saveChanges();
        delete mSection;
    }
    mSection = nullptr;

    rootObject->findChild<QObject*>("editMenuProvisionAll")->setProperty("visible", view == "provision");

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
    settings.remove("user/display_name");

    updateLoginStatus(false);

    mEngine->rootObjects().first()->findChild<QObject*>("helpMenuDeviceGuide")->setProperty("visible", true);

    emit loggedOut();

    mDeviceRechecks.clear();
    while(mDevices.size() > 0) {
        Serial::SerialDevice* device = mDevices.first();

        emit deviceChanged(device, false);
        mDevices.removeAt(0);

        mDeviceWorker->addDeviceRemove(device);
    }

    viewUpdate();
}

void UI::MainUI::importDevices(const QUrl& url) {
    QFile file(url.toLocalFile());
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QString data = QString(file.readAll());
    file.close();

    mDeviceWorker->addDeviceBatchLoad(data);
}

void UI::MainUI::loginStatusChanged(bool success) {
    qDebug()<<"loginStatusChanged"<<success;
    if(!success) {
        updateLoginStatus(false);

        QObject* rootObject = mEngine->rootObjects().first();
        QMetaObject::invokeMethod(rootObject->findChild<QObject*>("loginFailedContainer"), "show");

        return;
    }

    qInfo()<<"Logged in successfully!";

    mWorker->addDevicesListUpdate();

    updateLoginStatus(true);
}

void UI::MainUI::updateLoginStatus(bool loggedIn) {
    qDebug()<<"updateLoginStatus"<<loggedIn;

    QObject* rootObject = mEngine->rootObjects().first();

    rootObject->findChild<QObject*>("fileImportDevices")->setProperty("visible", loggedIn);
    rootObject->findChild<QObject*>("userMenu")->setProperty("visible", loggedIn);
    rootObject->findChild<QObject*>("editMenu")->setProperty("visible", loggedIn);
    // Show advanced menu item if we are using a testing build
#ifdef TESTING_MODE
    rootObject->findChild<QObject*>("advancedMenu")->setProperty("visible", loggedIn);
#endif

    if(loggedIn) {
        rootObject->findChild<QObject*>("usernameText")->setProperty("text", "");
        rootObject->findChild<QObject*>("passwordText")->setProperty("text", "");

        mEngine->rootContext()->setContextProperty("userDisplayName", QSettings().value("user/display_name").toString());

        QMetaObject::invokeMethod(rootObject->findChild<QObject*>("loginOverlay"), "hide");
        rootObject->findChild<QObject*>("loggingInOverlay")->setProperty("opacity", 0.0f);
    } else {
        QMetaObject::invokeMethod(rootObject->findChild<QObject*>("loginOverlay"), "show");

        mEngine->rootContext()->setContextProperty("userDisplayName", "");
    }
}

void UI::MainUI::versionUpdateCheck() {
    QObject* rootObject = mEngine->rootObjects().first();
    QMetaObject::invokeMethod(rootObject->findChild<QObject*>("loggingInView"), "setCheckForUpdates", Q_ARG(QVariant, true));
}

void UI::MainUI::versionUpdateAvailable(const QString& updaterDir, const QString &id, const QString &version, const QDateTime &updateTime) {
    qDebug()<<"New Update is available"<<version;

    QSettings updaterSettings(updaterDir + "/Updater.ini", QSettings::IniFormat);
    updaterSettings.setValue("path", QCoreApplication::applicationDirPath());
    updaterSettings.setValue("id", id);
    updaterSettings.setValue("version", version);
    updaterSettings.setValue("time", QString::number(updateTime.toTime_t()));
    updaterSettings.sync();

    QStringList argumentsList;
    argumentsList << "updater";

    QString upstFile = updaterDir + "/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    QProcess::startDetached(upstFile, argumentsList, updaterDir);

    mApp.quit();
}

void UI::MainUI::provisionProgressChanged(Serial::SerialDevice* device, int status, int current, int error) {
    Serial::SerialProvisionStatus provisionStatus = static_cast<Serial::SerialProvisionStatus>(status);
    Serial::SerialProvisionError provisionError = static_cast<Serial::SerialProvisionError>(error);

    if(provisionStatus == Serial::SerialProvisionStatusDone) {
        mWorker->addDeviceProvisionTracking(device, false);
    } else if(provisionStatus == Serial::SerialProvisionStatusError) {
        mWorker->addDeviceProvisionTracking(device, true, mLogHandler->getLogData());
    }

    mDevicesModel->setProgress(device, provisionStatus, current, provisionError);
}

void UI::MainUI::doRefresh() {
    mDeviceRechecks.clear();

    emit devicesRefresh();
}

void UI::MainUI::provisionAll() {
    mSection->saveChanges();

    foreach(Serial::SerialDevice* device, mDevices) {
        if(device->newMdn().size() == 0) {
            device->setProvisionData(device->mdn(), device->min());
        }

        if(!device->canProvision() || device->newMdn().size() == 0 || device->newMin() == static_cast<uint64_t>(-1)) {
            continue;
        }

        device->setProvisioning(true);
        provisionProgressChanged(device, Serial::SerialProvisionStatusQueue, 0, Serial::SerialProvisionErrorNone);
        viewUpdate();

        mDeviceWorker->addDeviceProvision(device);
    }
}

void UI::MainUI::provisionFailedClose() {
    qDebug()<<"provisionFailedClose";

    Serial::SerialDevice* device = mDevices.at(currentIndex());
    if(device == nullptr) {
        return;
    }

    device->setProvisioning(false);
    device->resetRTRE();
    mDevicesModel->setProgress(device, Serial::SerialProvisionStatusIdle, 0, Serial::SerialProvisionErrorNone);

    if(!device->isAvailable()) {
        deviceRemove(device->port());
    } else {
        viewUpdate();
    }
}

void UI::MainUI::updateFailedContinue() {
    emit loginStatusChanged(true);
}

UI::UISection::UISection(UI::MainUI* ui)
    : QObject(),
      mUI(ui)
{

}

void UI::UISection::beforeDeviceChanged() {

}

void UI::UISection::saveChanges() {

}

void UI::UISection::startUpdate() {
    QObject* rootObject = this->rootObject();

    QObject* currentDeviceLabel = rootObject->findChild<QObject*>("currentDeviceNameLabel");

    Serial::SerialDevice* device = currentDevice();
    if(device == nullptr) {
        QMetaObject::invokeMethod(currentDeviceLabel, "reset");
    } else {
        if(device->name().length() == 0) {
            currentDeviceLabel->setProperty("text", "Unknown");
        } else {
            currentDeviceLabel->setProperty("text", device->name());
        }
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
            rootObject->findChild<QObject*>("currentDeviceRTRE")->setProperty("value", currentDevice->rtreStr());
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
        rootObject->findChild<QObject*>("currentDeviceRTRE")->setProperty("value", "");
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
