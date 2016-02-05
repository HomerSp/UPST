#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QAbstractNativeEventFilter>
#include <QQmlContext>
#include <QScreen>

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
    Serial::SerialDevice* device = mDevices.at(index.row());
    switch(role) {
    case NameRole:
        return device->description();
    case PortRole:
        return device->port();
    }

    return "";
}
int ConnectedDevicesModel::rowCount(const QModelIndex &parent) const {
    return mDevices.size();
}

QHash<int, QByteArray> ConnectedDevicesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PortRole] = "port";
    return roles;
}

void ConnectedDevicesModel::deviceChanged(Serial::SerialDevice* device, bool added) {
    if(added) {
        QAbstractListModel::beginInsertRows(QModelIndex(), mDevices.size(), mDevices.size());
        mDevices.append(device);
        QAbstractListModel::endInsertRows();
    } else {
        for(int i = 0; i< mDevices.size(); i++) {
            if(mDevices[i] == device) {
                QAbstractListModel::beginRemoveRows(QModelIndex(), i, i);
                mDevices.removeAt(i);
                QAbstractListModel::endRemoveRows();
                break;
            }
        }
    }
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
    QString mdn = rootObject->findChild<QObject*>("textMDN")->property("text").toString();
    QString min = rootObject->findChild<QObject*>("textMIN")->property("text").toString();

    qDebug()<<"provision"<<mdn;

    /*if(mDevices.size() > 0) {
        mModel->addDevice(mDevices.at(0));
    }*/

    Serial::SerialDevice* device = mDevices.at(0);

    qDebug()<<"===== Resetting connection =====";

    QByteArray resetArray;
    resetArray.append((char)Serial::QCDM::Mode::MODE_RADIO_RESET);
    resetArray.append((char)0x0);
    Serial::QCDM::Commands::QcdmCommand resetCommand(device->communicator(), Serial::QCDM::DiagCommands::DIAG_CONTROL_F, resetArray);

    QList<QByteArray> result;
    if(!resetCommand.execute(result)) {
        qDebug()<<"Could not reset device";
    }

    return;

/*    Serial::SerialDevice* device = mDevices.at(0);

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
    }*/

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

    emit deviceChanged(device, true);

    viewUpdate();
}

void Main::deviceRemove(const QString& port) {
    for(int i = 0; i < mDevices.size(); i++) {
        if(*mDevices.at(i) == port) {
            Serial::SerialDevice* device = mDevices[i];
            emit deviceChanged(device, false);
            mDevices.removeAt(i);
            delete device;

            break;
        }
    }
}

void Main::currentDeviceChanged(int index) {
    viewUpdate();

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
    qDebug() << "viewUpdate";

    QObject* rootObject = mEngine->rootObjects().first();

    QObject* connectedDevicesList = rootObject->findChild<QObject*>("connectedDevicesList");
    int currentIndex = connectedDevicesList->property("currentIndex").toInt();
    if(currentIndex < 0 || currentIndex >= mDevices.size()) {
        return;
    }

    QObject* currentDeviceLabel = rootObject->findChild<QObject*>("currentDeviceNameLabel");
    if(mDevices.size() == 0) {
        QMetaObject::invokeMethod(currentDeviceLabel, "reset");
    } else {
        currentDeviceLabel->setProperty("text", mDevices[currentIndex]->description());
    }

    QObject* pageLoader = rootObject->findChild<QObject*>("mainPageLoader");

    QString view = pageLoader->property("currentView").toString();
    if(view == "manual") {

    } else if(view == "provision") {
        if(mDevices.size() > 0) {
            Serial::SerialDevice* currentDevice = mDevices.at(currentIndex);
            rootObject->findChild<QObject*>("currentDeviceMDN")->setProperty("value", currentDevice->mdn());
            rootObject->findChild<QObject*>("currentDeviceMIN")->setProperty("value", QString("%1").arg(currentDevice->min(), 10, 10, QChar('0')));
            rootObject->findChild<QObject*>("currentDeviceESN")->setProperty("value", QString("%1").arg(currentDevice->esn(), 8, 16, QChar('0')));
            rootObject->findChild<QObject*>("currentDeviceMEID")->setProperty("value", QString("%1").arg(currentDevice->meid(), 14, 16, QChar('0')));
            rootObject->findChild<QObject*>("currentDeviceIMEI")->setProperty("value", QString("%1").arg(currentDevice->imei(), 14, 16, QChar('0')));
        }
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    ConnectedDevicesModel devicesModel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("devicesModel", &devicesModel);

    engine.load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    Main main(&engine);
    QObject::connect(&main, &Main::deviceChanged, &devicesModel, &ConnectedDevicesModel::deviceChanged);

    DeviceFilterEvent deviceFilter;
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceAdd, &main, &Main::deviceAdd);
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceRemove, &main, &Main::deviceRemove);

    app.installNativeEventFilter(&deviceFilter);

    deviceFilter.refresh();

    return app.exec();
}

