#include <QObject>
#include <QDebug>
#include <QFile>

#include "serialcommunicator.h"
#include "serialdevice.h"
#include "serialprovisiondata.h"
#include "qcdm/nv/nvprovisiondata.h"
#include "qcdm/commands/nvcommands/esncommand.h"
#include "qcdm/commands/nvcommands/imeicommand.h"
#include "qcdm/commands/nvcommands/meidcommand.h"
#include "qcdm/commands/nvcommands/mdncommand.h"
#include "qcdm/commands/nvcommands/mincommand.h"

using namespace Serial;

SerialDevice::SerialDevice(const QString& port, uint16_t vid, uint16_t pid)
    : mCommunicator(nullptr),
      mPort(port), mVid(vid), mPid(pid),
      mBaudRate(QSerialPort::Baud115200),
      mDataBits(QSerialPort::Data8),
      mParity(QSerialPort::NoParity),
      mStopBits(QSerialPort::OneStop),
      mMake(""),
      mModel(""),
      mMdn(""),
      mMin(0),
      mESN(0),
      mIMEI(0),
      mMEID(0)

{
    mCommunicator = new SerialCommunicator(*this);
    mCommunicator->open();
    mCommunicator->clear();
}

SerialDevice::SerialDevice(const QSerialPortInfo& info)
    : SerialDevice(info.portName(), info.vendorIdentifier(), info.productIdentifier()) {

}

SerialDevice::~SerialDevice() {
    foreach(SerialDevice* c, mChildren) {
        delete c;
    }

    if(mCommunicator != nullptr) {
        delete mCommunicator;
    }

    mCommunicator = nullptr;
}

SerialCommunicator* SerialDevice::communicator() {
    return mCommunicator;
}

void SerialDevice::addChild(SerialDevice *device) {
    mChildren.append(device);
}

bool SerialDevice::isSameDevice(SerialDevice *device) {
    return mVid == device->mVid && mPid == device->mPid && mMEID == device->mMEID;
}

bool SerialDevice::isValid() {
    Serial::QCDM::Commands::QcdmCommand cmd(this, Serial::QCDM::DiagCommands::DIAG_VERNO_F);
    cmd.setTimeout(1000);

    cmd.execute();

    return cmd.result()->success();
}

bool SerialDevice::provision() {
    bool ret = true;

    QFile sourceFile(":/res/data/provision_data.json");
    sourceFile.open(QFile::ReadOnly | QFile::Text);

    SerialProvisionData* provisionData = new Serial::QCDM::Nv::NvProvisionData(this, sourceFile.readAll());

    qDebug()<<"===== Writing SPC =====";
    Serial::QCDM::Commands::QcdmCommand spcCommand(this, Serial::QCDM::DiagCommands::DIAG_SPC_F, provisionData->carrierSPC().toLatin1());
    spcCommand.execute();
    if(spcCommand.result()->success()) {
        qDebug()<<"Result="<<spcCommand.result()->data().toString();
    } else {
        qDebug()<<"Could not unlock SPC";
        ret = false;
    }

    qDebug()<<"===== WRITING MDN =====";
    Serial::QCDM::Commands::Nv::MDNCommand mdnCmd(this, false, mMdn);
    mdnCmd.execute();
    if(mdnCmd.result()->success()) {
        qDebug()<<"Result="<<mdnCmd.result()->data().toString();
    } else {
        qDebug()<<"Could not write MDN";
        ret = false;
    }

    qDebug()<<"===== WRITING MIN =====";
    Serial::QCDM::Commands::Nv::MINCommand minCmd(this, false, mMin);
    minCmd.execute();
    if(minCmd.result()->success()) {
        qDebug()<<"Result="<<minCmd.result()->data().toString();
    } else {
        qDebug()<<"Could not write MIN";
        ret = false;
    }

    foreach(Serial::SerialCommand* cmd, provisionData->constCommands()) {
#ifdef TESTING_MODE
        qDebug()<<"Provisioning"<<cmd->debuggingName();
#endif
        // Increase the timeout period
        cmd->setTimeout(10000);
        cmd->execute();
        foreach(Serial::SerialCommandResult* result, cmd->results()) {
           if(!result->success()) {
               qDebug()<<"Failed to provision";
           } else {
               qDebug()<<"Provision success";
           }
        }
    }

    delete provisionData;

    return ret;
}

bool SerialDevice::update() {
    qDebug()<<"===== GETTING ESN =====";
    Serial::QCDM::Commands::Nv::ESNCommand esnCmd(this);
    esnCmd.execute();
    if(esnCmd.result()->success()) {
        mESN = esnCmd.result()->data().toUInt();
    }

    qDebug()<<"===== GETTING IMEI =====";
    Serial::QCDM::Commands::Nv::IMEICommand imeiCmd(this);
    imeiCmd.execute();
    if(imeiCmd.result()->success()) {
        mIMEI = imeiCmd.result()->data().toULongLong();
    }

    qDebug()<<"===== GETTING MEID =====";
    Serial::QCDM::Commands::Nv::MEIDCommand meidCmd(this);
    meidCmd.execute();
    if(meidCmd.result()->success()) {
        mMEID = meidCmd.result()->data().toULongLong();
    }

    qDebug()<<"===== GETTING MDN =====";
    Serial::QCDM::Commands::Nv::MDNCommand mdnCmd(this);
    mdnCmd.execute();
    if(mdnCmd.result()->success()) {
        mMdn = mdnCmd.result()->data().toString();
    }

    qDebug()<<"===== GETTING MIN =====";
    Serial::QCDM::Commands::Nv::MINCommand minCmd(this);
    minCmd.execute();
    if(minCmd.result()->success()) {
        mMin = minCmd.result()->data().toULongLong();
    }

    qDebug()<<"ESN:"<<QString::number(mESN, 16)<<"IMEI:"<<QString::number(mIMEI, 16)<<"MEID:"<<QString::number(mMEID, 16)<<"MDN:"<<mMdn<<", MIN:"<<mMin;

    return true;
}

bool SerialDevice::updateJson(const QJsonObject& obj) {
    if(obj.contains("make")) {
        mMake = obj["make"].toString();
    }
    if(obj.contains("model")) {
        mModel = obj["model"].toString();
    }
    if(obj.contains("codename")) {
        mCodename = obj["codename"].toString();
    }

    return true;
}

bool SerialDevice::operator==(const SerialDevice& other) {
    if(*this == other.mPort) {
        return true;
    }

    foreach(Serial::SerialDevice* c, other.mChildren) {
        if(*this == c->mPort) {
            return true;
        }
    }

    return false;
}

bool SerialDevice::operator==(const QString& port) {
    if(port == mPort) {
        return true;
    }

    foreach(Serial::SerialDevice* c, mChildren) {
        if(c->mPort == port) {
            return true;
        }
    }

    return false;
}
