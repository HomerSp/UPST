#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QAbstractNativeEventFilter>
#include <QQmlContext>

#include "main.h"
#include "devicefilterevent.h"

#include "serial/serialcommunicator.h"
#include "serial/qcdm/diag.h"
#include "serial/qcdm/commands/nv/mdncommand.h"
#include "serial/qcdm/commands/nv/mob_modelcommand.h"
#include "serial/qcdm/commands/nv/mob_firmwarerev.h"
#include "serial/qcdm/commands/nv/imeicommand.h"
#include "serial/crcutils.h"

ConnectedDevicesModel::ConnectedDevicesModel(QObject* parent)
    : QAbstractListModel(parent) {

}

QVariant ConnectedDevicesModel::data(const QModelIndex& index, int role) const {
    return mDevices.at(index.row());
}
int ConnectedDevicesModel::rowCount(const QModelIndex &parent) const {
    return mDevices.size();
}

QHash<int, QByteArray> ConnectedDevicesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    return roles;
}

void ConnectedDevicesModel::addDevice(const QString& name) {
    mDevices.append(name);
}

Main::Main(QQmlApplicationEngine* engine)
    : mEngine(engine)
{
    QObject* rootObject = engine->rootObjects().first();

    // Hide advanced menu item if we are not using a debug build
#ifndef QT_DEBUG
    QObject* advancedMenuObject = rootObject->findChild<QObject*>("advancedMenu");
    advancedMenuObject->setProperty("visible", false);
#endif

    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");
    QObject::connect(pageLoader, SIGNAL(viewChanged()), this, SLOT(viewChanged()));

    QObject* connectedDevicesList = rootObject->findChild<QObject*>("connectedDevicesList");
    QObject::connect(connectedDevicesList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentDeviceChanged(int)));

    viewChanged();
    currentDeviceChanged(connectedDevicesList->property("currentIndex").toInt());
}

Main::~Main() {
    foreach(Serial::SerialDevice* d, mDevices) {
        delete d;
    }

    mDevices.clear();
}

void Main::provision() {
    QObject* rootObject = mEngine->rootObjects().first();
    QObject* textMDN = rootObject->findChild<QObject*>("textMDN");
    QString mdn = textMDN->property("text").toString();

    qDebug()<<"provision"<<mdn;

    return;

    Serial::SerialDevice* device = mDevices.at(0);

    qDebug()<<"===== Writing SPC =====";
    Serial::QCDM::Commands::QcdmCommand spcCommand(device->communicator(), Serial::QCDM::DiagCommands::DIAG_SPC_F, QString("000000").toLatin1());

    QByteArray result;
    if(!spcCommand.execute(result)) {
        qDebug()<<"Could not unlock SPC";
    }

    qDebug()<<"===== Setting NV-only =====";

    Serial::QCDM::Commands::Nv::NvCommand8Bit nvOnlySwitch(device->communicator(), Serial::QCDM::DiagCommands::DIAG_NV_WRITE_F, Serial::QCDM::NvItem::NV_RTRE_CONFIG_I, 0);
    //Serial::QCDM::Commands::Nv::NvCommand8Bit nvOnlySwitch(device->communicator(), Serial::QCDM::DiagCommands::DIAG_NV_READ_F, Serial::QCDM::NvItem::NV_RTRE_CONFIG_I);

    uint8_t res;
    if(!nvOnlySwitch.execute(res)) {
        qDebug()<<"Could not get nv only switch";
    }

    qDebug()<<"NV-only mode is"<<res;

    qDebug()<<"===== Getting model =====";

    Serial::QCDM::Commands::Nv::NvCommand32Bit mobModel(device->communicator(), Serial::QCDM::DiagCommands::DIAG_NV_READ_F, Serial::QCDM::NvItem::NV_MOB_MODEL_I);

    uint32_t model;
    if(!mobModel.execute(model)) {
        qDebug()<<"Could not get model from device"<<device->port();
    }

    qDebug()<<"Model is"<<model;

    qDebug()<<"===== Writing MDN =====";

    Serial::QCDM::Commands::Nv::MDNCommand mdnCmd(device->communicator(), Serial::QCDM::DiagCommands::DIAG_NV_WRITE_F, mdn);

    QString ret;
    if(!mdnCmd.execute(ret)) {
        qDebug()<<"Could not get MDN from device"<<device->port();
    }

    /*qDebug()<<"===== Resetting connection =====";

    QByteArray resetArray;
    resetArray.append((char)Serial::QCDM::Mode::MODE_RADIO_RESET);
    resetArray.append((char)0x0);
    Serial::QCDM::Commands::QcdmCommand resetCommand(device->communicator(), Serial::QCDM::DiagCommands::DIAG_CONTROL_F, resetArray);
    if(!resetCommand.execute(result)) {
        qDebug()<<"Could not reset device";
    }*/

}

void Main::deviceAdd(Serial::SerialDevice* device) {
    qDebug()<<"deviceAdd"<<device->port();

    foreach(Serial::SerialDevice* d, mDevices) {
        if(*d == *device) {
            delete device;
            return;
        }
    }

    mDevices.append(device);

    device->update();

    viewUpdate();

    emit deviceChanged(*device, true);
}

void Main::deviceRemove(const QString& port) {
    for(int i = 0; i < mDevices.size(); i++) {
        if(*mDevices.at(i) == port) {
            Serial::SerialDevice* device = mDevices[i];
            emit deviceChanged(*device, false);
            mDevices.removeAt(i);

            break;
        }
    }
}

void Main::currentDeviceChanged(int index) {
    qDebug()<<"currentDeviceChanged"<<index;

    QString device = "";
    switch(index) {
    case 0:
        device = "SAMSUNG GT-i9100";
        break;
    default:
        device = "HTC Rezound";
        break;
    }

    QObject* rootObject = mEngine->rootObjects().first();
    QObject* currentDeviceLabel = rootObject->findChild<QObject*>("currentDeviceNameLabel");
    currentDeviceLabel->setProperty("text", device);
}

void Main::viewChanged() {
    QObject* rootObject = mEngine->rootObjects().first();
    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");

    QString view = pageLoader->property("currentView").toString();
    qDebug()<<"viewChanged"<<view;

    if(view == "manual") {

    } else if(view == "provision") {
        QObject* provisionButton = rootObject->findChild<QObject*>("provisionButton");
        QObject::connect(provisionButton, SIGNAL(clicked()), this, SLOT(provision()));

        viewUpdate();
    }
}

void Main::viewUpdate() {
    QObject* rootObject = mEngine->rootObjects().first();
    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");

    QString view = pageLoader->property("currentView").toString();
    if(view == "manual") {

    } else if(view == "provision") {
        if(mDevices.size() > 0) {
            QObject* textMDN = rootObject->findChild<QObject*>("textMDN");
            textMDN->setProperty("text", mDevices.at(0)->mdn());
        }
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    ConnectedDevicesModel devicesModel;
    devicesModel.addDevice("SAMSUNG GT-i9100");
    devicesModel.addDevice("HTC Rezound");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("devicesModel", &devicesModel);
    engine.load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    Main main(&engine);

    DeviceFilterEvent deviceFilter;
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceAdd, &main, &Main::deviceAdd);
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceRemove, &main, &Main::deviceRemove);

    app.installNativeEventFilter(&deviceFilter);

    deviceFilter.refresh();

    return app.exec();
}

