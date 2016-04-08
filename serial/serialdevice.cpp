#include <QObject>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QDataStream>

#include "../web/webutils.h"
#include "serialcommunicator.h"
#include "serialdevice.h"
#include "serialprovisiondata.h"
#include "qcdm/nv/nvprovisiondata.h"
#include "qcdm/commands/nvcommands/esncommand.h"
#include "qcdm/commands/nvcommands/imeicommand.h"
#include "qcdm/commands/nvcommands/meidcommand.h"
#include "qcdm/commands/nvcommands/mdncommand.h"
#include "qcdm/commands/nvcommands/mincommand.h"
#include "qcdm/commands/prlcommand.h"
#include "qcdm/commands/radiomodecommand.h"

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
      mType(SerialDeviceTypeUnknown),
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
    emit provisionProgressChanged(1, 0);

    QFile sourceFile(":/res/data/provision_data.json");
    if(!sourceFile.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    emit provisionProgressChanged(1, 1);

    SerialProvisionData* provisionData = new Serial::QCDM::Nv::NvProvisionData(this, sourceFile.readAll());

    emit provisionProgressChanged(1, 2);

    bool ret = provision(provisionData);
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
    if(obj.contains("type")) {
        QString type = obj["type"].toString();
        if(type == "smartphone") {
            mType = SerialDeviceTypeSmartphone;
        } else if(type == "featurephone") {
            mType = SerialDeviceTypeFeaturePhone;
        } else if(type == "tablet") {
            mType = SerialDeviceTypeTablet;
        } else if(type == "mifi") {
            mType = SerialDeviceTypeMifi;
        }
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

bool SerialDevice::provision(SerialProvisionData* data) {
    bool ret = true;

    QList<SerialDevice*> devices;
    devices.append(this);
    devices.append(mChildren);

    float mod = (98.0f / (data->constCommands().size() + 5)) / devices.size();
    float i = 2;

    foreach(SerialDevice* device, devices) {
        qDebug()<<"===== Provisioning port"<<device->communicator()->port()<<"=====";

        qDebug()<<"===== RESETTING DEVICE BEFORE =====";
        Serial::QCDM::Commands::RadioModeCommand radioCmdBefore(device, Serial::QCDM::MODE_RADIO_OFFLINE);
        radioCmdBefore.execute();
        if(!radioCmdBefore.resultSuccess()) {
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(1, i);

        if(data->password16().size() == 16) {
            qDebug()<<"===== Sending password =====";
            Serial::QCDM::Commands::QcdmCommand passwordCmd(device, Serial::QCDM::DIAG_PASSWORD_F, data->password16().toLatin1());
            passwordCmd.execute();
            if(passwordCmd.result()->success()) {
                qDebug()<<"Result="<<passwordCmd.result()->data().toString();
            } else {
                qDebug()<<"Could not send password";
                ret = false;
                break;
            }
        }

        qDebug()<<"===== Writing SPC =====";
        Serial::QCDM::Commands::QcdmCommand spcCommand(device, Serial::QCDM::DiagCommands::DIAG_SPC_F, data->carrierSPC().toLatin1());
        spcCommand.execute();
        if(spcCommand.result()->success()) {
            qDebug()<<"Result="<<spcCommand.result()->data().toString();
        } else {
            qDebug()<<"Could not unlock SPC";
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(1, i);

        qDebug()<<"===== WRITING MDN =====";
        Serial::QCDM::Commands::Nv::MDNCommand mdnCmd(device, false, mMdn);
        mdnCmd.execute();
        if(mdnCmd.result()->success()) {
            qDebug()<<"Result="<<mdnCmd.result()->data().toString();
        } else {
            qDebug()<<"Could not write MDN";
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(1, i);

        qDebug()<<"===== WRITING MIN =====";
        Serial::QCDM::Commands::Nv::MINCommand minCmd(device, false, mMin);
        minCmd.execute();
        if(minCmd.result()->success()) {
            qDebug()<<"Result="<<minCmd.result()->data().toString();
        } else {
            qDebug()<<"Could not write MIN";
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(1, i);

        qDebug()<<"Provision items"<<data->constCommands().size();

        foreach(Serial::SerialCommand* cmd, data->constCommands()) {
            if(!provision(device, data, cmd)) {
                ret = false;
                break;
            }

            i += mod;
            emit provisionProgressChanged(1, i);
        }

        if(!ret) {
            break;
        }

        // We need to reset the command results for the next device.
        data->resetCommands();

        qDebug()<<"===== RESETTING DEVICE AFTER =====";
        Serial::QCDM::Commands::RadioModeCommand radioCmdAfter(device, Serial::QCDM::MODE_RADIO_OFFLINE);
        radioCmdAfter.execute();
        if(!radioCmdAfter.resultSuccess()) {
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(1, i);
    }

    if(ret) {
        emit provisionProgressChanged(2, 100);
    } else {
        emit provisionProgressChanged(3, i);
    }

    return ret;
}

bool SerialDevice::provision(SerialDevice* device, SerialProvisionData* data, SerialCommand* cmd) {
#ifdef TESTING_MODE
    qDebug()<<"Provisioning"<<cmd->debuggingName();
#endif

    if(data->sequentialOffline()) {
        if(data->password16().size() == 16) {
            Serial::QCDM::Commands::QcdmCommand passwordCmd(device, Serial::QCDM::DIAG_PASSWORD_F, data->password16().toLatin1());
            passwordCmd.execute();
            if(!passwordCmd.resultSuccess()) {
                return false;
            }
        }

        Serial::QCDM::Commands::QcdmCommand spcCommand(device, Serial::QCDM::DiagCommands::DIAG_SPC_F, data->carrierSPC().toLatin1());
        spcCommand.execute();
        if(!spcCommand.resultSuccess()) {
            return false;
        }
    }

    // Increase the timeout period
    cmd->setTimeout(10000);
    cmd->execute(device);
    foreach(Serial::SerialCommandResult* result, cmd->results()) {
        if(!result->success()) {
            qDebug()<<"Failed to provision";
            return false;
        } else {
            qDebug()<<"Provision success";
        }
    }

    if(data->sequentialOffline()) {
        Serial::QCDM::Commands::RadioModeCommand radioCmd(device, Serial::QCDM::MODE_RADIO_OFFLINE);
        radioCmd.execute();
        if(!radioCmd.resultSuccess()) {
            return false;
        }
    }

    return true;
}
