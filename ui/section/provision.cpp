#include <QDebug>

#include "../../serial/qcdm/commands/qcdmcommand.h"

#include "provision.h"

UI::Section::Provision::Provision(UI::MainUI* ui)
    : UISection(ui)
{
    QObject* provisionButton = rootObject()->findChild<QObject*>("provisionButton");
    QObject::connect(provisionButton, SIGNAL(clicked()), this, SLOT(provision()));
}

UI::Section::Provision::~Provision() {

}

void UI::Section::Provision::update() {
    UISection::startUpdate();

    QObject* rootObject = UISection::rootObject();

    Serial::SerialDevice* currentDevice = UISection::currentDevice();
    if(currentDevice != nullptr) {
        rootObject->findChild<QObject*>("currentDeviceMake")->setProperty("value", currentDevice->makeStr());
        rootObject->findChild<QObject*>("currentDeviceModel")->setProperty("value", currentDevice->modelStr());
        rootObject->findChild<QObject*>("currentDeviceMDN")->setProperty("value", currentDevice->mdnStr());
        rootObject->findChild<QObject*>("currentDeviceMIN")->setProperty("value", currentDevice->minStr());
        rootObject->findChild<QObject*>("currentDeviceESN")->setProperty("value", currentDevice->esnStr());
        rootObject->findChild<QObject*>("currentDeviceMEID")->setProperty("value", currentDevice->meidStr());
        rootObject->findChild<QObject*>("currentDeviceIMEI")->setProperty("value", currentDevice->imeiStr());
    } else {
        rootObject->findChild<QObject*>("currentDeviceMake")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceModel")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceMDN")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceMIN")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceESN")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceMEID")->setProperty("value", "");
        rootObject->findChild<QObject*>("currentDeviceIMEI")->setProperty("value", "");
    }

    UISection::endUpdate();
}

void UI::Section::Provision::provision() {
    QObject* rootObject = UISection::rootObject();
    QString mdn = rootObject->findChild<QObject*>("textMDN")->property("text").toString();
    QString min = rootObject->findChild<QObject*>("textMIN")->property("text").toString();

    qDebug()<<"provision"<<mdn;

    /*if(mDevices.size() > 0) {
        mModel->addDevice(mDevices.at(0));
    }*/

    Serial::SerialDevice* device = currentDevice();
    if(device == nullptr) {
        return;
    }

    qDebug()<<"===== Resetting connection =====";

    QByteArray resetArray;
    resetArray.append((char)Serial::QCDM::Mode::MODE_RADIO_RESET);
    resetArray.append((char)0x0);
    Serial::QCDM::Commands::QcdmCommand resetCommand(device, Serial::QCDM::DiagCommands::DIAG_CONTROL_F, resetArray);

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
