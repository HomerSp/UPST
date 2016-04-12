#include <QDebug>
#include <QQmlContext>
#include <QQmlProperty>
#include <QThread>

#include "ui.h"
#include "section/deviceinfo.h"
#include "section/manual.h"
#include "section/provision.h"

UI::MainUI::MainUI(const QGuiApplication& app)
    : QObject(),
      mApp(app),
      mSection(nullptr)
{
    mDevicesModel = new UI::ConnectedDevicesModel();
    QObject::connect(this, &UI::MainUI::deviceChanged, mDevicesModel, &UI::ConnectedDevicesModel::deviceChanged);
    QObject::connect(this, &UI::MainUI::deviceUpdate, mDevicesModel, &UI::ConnectedDevicesModel::deviceUpdate);

    mEngine = new QQmlApplicationEngine();
    mEngine->rootContext()->setContextProperty("programVersion", QString(PROG_VERSION));
    mEngine->rootContext()->setContextProperty("qtVersion", QString(QT_VERSION_STR));
    mEngine->rootContext()->setContextProperty("devicesModel", mDevicesModel);

    QObject::connect(mEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    mEngine->load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

    /* Set up signals */
    QObject* rootObject = mEngine->rootObjects().first();

    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");
    QObject::connect(pageLoader, SIGNAL(viewChanged()), this, SLOT(viewChanged()));

    QObject* connectedDevicesList = rootObject->findChild<QObject*>("connectedDevicesList");
    QObject::connect(connectedDevicesList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentDeviceChanged(int)));
    QObject::connect(connectedDevicesList, SIGNAL(refresh()), this, SLOT(devicesChanged()));

    QObject::connect(rootObject->findChild<QObject*>("loginButton"), SIGNAL(clicked()), this, SLOT(login()));
    QObject::connect(rootObject->findChild<QObject*>("fileMenuLogout"), SIGNAL(triggered()), this, SLOT(logout()));

    viewChanged();
    currentDeviceChanged(connectedDevicesList->property("currentIndex").toInt());

    QThread* thread = new QThread;

    mWorker = new SerialDeviceWorker();
    mWorker->moveToThread(thread);

    connect(mWorker, &SerialDeviceWorker::deviceAdd, this, &MainUI::deviceAdd);
    connect(mWorker, &SerialDeviceWorker::loginStatus, this, &MainUI::loginStatusChanged);
    connect(mWorker, &SerialDeviceWorker::statusChange, this, &MainUI::setStatus);
    connect(mWorker, &SerialDeviceWorker::provisionProgressChanged, mDevicesModel, &ConnectedDevicesModel::setProgress);

    connect(thread, SIGNAL(started()), mWorker, SLOT(process()));
    connect(mWorker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(mWorker, SIGNAL(finished()), mWorker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), mWorker, SLOT(deleteLater()));

    thread->start();
}

UI::MainUI::~MainUI() {
    if(mSection != nullptr) {
        delete mSection;
    }

    mWorker->stop();

    delete mEngine;
    delete mDevicesModel;
}

void UI::MainUI::devicesChanged() {
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
            mWorker->addDeviceCheck(info);
        } else {
            qDebug()<<"Not checking port"<<port;
        }
    }
}

void UI::MainUI::deviceAdd(Serial::SerialDevice* device) {
    qDebug()<<"deviceAdd"<<device->port();

    // Do we already have this device?
    foreach(Serial::SerialDevice* d, mDevices) {
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

    mDevices.append(device);

    emit deviceChanged(device, true);

    viewUpdate();
}

void UI::MainUI::deviceRemove(const QString& port) {
    for(int i = 0; i < mDevices.size(); i++) {
        if(*mDevices.at(i) == port) {
            Serial::SerialDevice* device = mDevices[i];
            emit deviceChanged(device, false);
            mDevices.removeAt(i);

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
    mWorker->addLogin("", "");
}

void UI::MainUI::logout() {
    updateLoginStatus(false);

    emit loggedOut();

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        deviceRemove(info.portName());
    }
}

void UI::MainUI::loginStatusChanged(bool success) {
    if(!success) {
        // TODO: Display an error.
        return;
    }

    updateLoginStatus(true);

    emit loggedIn();
}

void UI::MainUI::updateLoginStatus(bool loggedIn) {
    QObject* rootObject = mEngine->rootObjects().first();

    rootObject->findChild<QObject*>("fileMenuLogout")->setProperty("visible", loggedIn);
    rootObject->findChild<QObject*>("editMenu")->setProperty("visible", loggedIn);
    // Show advanced menu item if we are using a testing build
#ifdef TESTING_MODE
    rootObject->findChild<QObject*>("advancedMenu")->setProperty("visible", loggedIn);
#endif


    rootObject->findChild<QObject*>("loginOverlay")->setProperty("opacity", (loggedIn)?0:1);
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
