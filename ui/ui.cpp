#include <QDebug>
#include <QQmlContext>
#include <QThread>

#include "ui.h"
#include "section/manual.h"
#include "section/provision.h"

UI::MainUI::MainUI(const QGuiApplication& app)
    : QObject(),
      mApp(app),
      mSection(nullptr)
{
    mDevicesModel = new UI::ConnectedDevicesModel();
    QObject::connect(this, &UI::MainUI::deviceChanged, mDevicesModel, &UI::ConnectedDevicesModel::deviceChanged);

    mEngine = new QQmlApplicationEngine();
    mEngine->rootContext()->setContextProperty("devicesModel", mDevicesModel);

    QObject::connect(mEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    mEngine->load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

    /* Set up signals */
    QObject* rootObject = mEngine->rootObjects().first();

    // Hide advanced menu item if we are not using a testing build
#ifndef TESTING_MODE
    QObject* advancedMenuObject = rootObject->findChild<QObject*>("advancedMenu");
    advancedMenuObject->setProperty("visible", false);
#endif

    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");
    QObject::connect(pageLoader, SIGNAL(viewChanged()), this, SLOT(viewChanged()));

    QObject* connectedDevicesList = rootObject->findChild<QObject*>("connectedDevicesList");
    QObject::connect(connectedDevicesList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentDeviceChanged(int)));

    viewChanged();
    currentDeviceChanged(connectedDevicesList->property("currentIndex").toInt());

    QThread* thread = new QThread;

    mWorker = new SerialDeviceWorker();
    mWorker->moveToThread(thread);

    connect(mWorker, &SerialDeviceWorker::deviceAdd, this, &MainUI::deviceAdd);
    connect(mWorker, &SerialDeviceWorker::statusChange, this, &MainUI::setStatus);

    connect(thread, SIGNAL(started()), mWorker, SLOT(process()));
    connect(mWorker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(mWorker, SIGNAL(finished()), mWorker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), mWorker, SLOT(deleteLater()));

    thread->start();
}

UI::MainUI::~MainUI() {
    mWorker->stop();

    delete mEngine;
    delete mDevicesModel;

    if(mSection != nullptr) {
        delete mSection;
    }
}

void UI::MainUI::devicesChanged() {
    setStatus("Refreshing devices");

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
            mWorker->addNewDevice(info);
        }
    }
}

void UI::MainUI::deviceAdd(Serial::SerialDevice* device) {
    qDebug()<<"deviceAdd"<<device->port();

    foreach(Serial::SerialDevice* d, mDevices) {
        if(*d == *device) {
            delete device;
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

            mWorker->removeDevice(device);

            break;
        }
    }

    viewUpdate();
}

void UI::MainUI::currentDeviceChanged(int index) {
    Q_UNUSED(index);

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

UI::UISection::UISection(UI::MainUI* ui)
    : QObject(),
      mUI(ui)
{

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
    }
}

void UI::UISection::endUpdate() {
    if(devices().size() > 0) {
        QObject* rootObject = this->rootObject();

        QObject* bottomTab = rootObject->findChild<QObject*>("mainPageBottomTabArrow");
        bottomTab->setProperty("enabled", true);
        bottomTab->setProperty("hidden", false);

        rootObject->findChild<QObject*>("noDeviceOverlay")->setProperty("opacity", 0.0f);
    }
}

Serial::SerialDevice* UI::UISection::currentDevice() {
    QObject* connectedDevicesList = rootObject()->findChild<QObject*>("connectedDevicesList");
    int currentIndex = connectedDevicesList->property("currentIndex").toInt();

    // The index defaults to -1, so we check the first item if it's the default.
    if(currentIndex < 0) {
        currentIndex = 0;
    }
    if(currentIndex >= devices().size()) {
        return nullptr;
    }

    return devices().at(currentIndex);
}
