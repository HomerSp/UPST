#include <QObject>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QDataStream>
#include <QThread>

#include "web/webutils.h"
#include "serialcommunicator.h"
#include "serialdevice.h"
#include "serialprovisiondata.h"
#include "qcdm/nv/nvprovisiondata.h"
#include "qcdm/commands/nvcommands/esncommand.h"
#include "qcdm/commands/nvcommands/imeicommand.h"
#include "qcdm/commands/nvcommands/meidcommand.h"
#include "qcdm/commands/nvcommands/mdncommand.h"
#include "qcdm/commands/nvcommands/mincommand.h"
#include "qcdm/commands/passwordcommand.h"
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
      mID(""),
      mMake(""),
      mModel(""),
      mType(SerialDeviceTypeUnknown),
      mFlags(0),
      mMdn(""),
      mMin(0),
      mESN(0),
      mIMEI(0),
      mMEID(0),
      mProvisioning(false),
      mProvisionStop(false),
      mNewMdn(""),
      mNewMin(0)

{
    mCommunicator = new SerialCommunicator(*this);
    if(mCommunicator->open()) {
        mCommunicator->clear();
    }
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

bool SerialDevice::open() {
    bool ret = mCommunicator->open();
    if(!ret) {
        return false;
    }

    foreach(Serial::SerialDevice* c, mChildren) {
        ret = ret || c->open();
    }

    return ret;
}

bool SerialDevice::close() {
    bool ret = mCommunicator->close();
    if(!ret) {
        return false;
    }

    foreach(Serial::SerialDevice* c, mChildren) {
        if(!c->close()) {
            ret = false;
        }
    }

    return ret;
}

void SerialDevice::addChild(SerialDevice *device) {
    mChildren.append(device);
}

bool SerialDevice::isAvailable() {
    return mCommunicator != nullptr && mCommunicator->isOpen();
}

bool SerialDevice::isSameDevice(SerialDevice *device) {
    if(mVid == device->mVid && mPid == device->mPid) {
        if(mMEID != 0 && mMEID == device->mMEID) {
            return true;
        }
        if(mIMEI != 0 && mIMEI == device->mIMEI) {
            return true;
        }
        // We only ever want to check the ESN if both the IMEI and MEID are empty.
        if(mMEID == 0 && mIMEI == 0 && mESN != 0 && mESN == device->mESN) {
            return true;
        }
    }

    return false;
}

bool SerialDevice::isValid() {
    Serial::QCDM::Commands::QcdmCommand cmd(this, Serial::QCDM::DiagCommands::DIAG_VERNO_F);
    cmd.setTimeout(1000);

    cmd.execute();

    return cmd.result()->success();
}

bool SerialDevice::canProvision() {
    return isAvailable() && mType != Serial::SerialDeviceTypeUnknown && !isProvisioning() && (!flagMultiPort() || (flagMultiPort() && mChildren.size() > 0));
}

bool SerialDevice::provision(const QString& userToken) {
    mProvisionStop = false;

    qInfo()<<"Provisioning"<<name();

    emit provisionProgressChanged(SerialProvisionStatusProgress, 0);

    QByteArray output;
    QHash<QString, QString> headers;
    headers.insert("U-Token", userToken);

    QString postData = "d=" + mID;

    qDebug()<<"Downloading provisioning data for"<<mID;

    if(!Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/provision.php"), output, headers, postData)) {
        qCritical()<<"Failed to download provision data";
        emit provisionProgressChanged(SerialProvisionStatusError, 0, SerialProvisionErrorDownload);
        return false;
    }

    emit provisionProgressChanged(SerialProvisionStatusProgress, 1);

    SerialProvisionData* provisionData = new Serial::QCDM::Nv::NvProvisionData(this, QString(output));
    if(!provisionData->valid()) {
        qCritical()<<"Failed to parse provision data";
        emit provisionProgressChanged(SerialProvisionStatusError, 1, SerialProvisionErrorData);
        delete provisionData;
        return false;
    }

    emit provisionProgressChanged(SerialProvisionStatusProgress, 2);

    qDebug()<<"Provision process started at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

    bool ret = provision(provisionData);

    qDebug()<<"Provision process finished at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

    delete provisionData;

    return ret;
}

bool SerialDevice::update(bool* reschedule) {
    qDebug()<<"===== GETTING ESN =====";
    Serial::QCDM::Commands::Nv::ESNCommand esnCmd(this);
    esnCmd.execute();
    if(esnCmd.result()->success()) {
        mESN = esnCmd.result()->data().toUInt();
    } else {
        // Could not retrieve ESN, reschedule the device check.
        if(reschedule != nullptr) {
            *reschedule = true;
            return false;
        }
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
    if(obj.contains("make") && obj.contains("model") && obj.contains("type")) {
        QString type = obj["type"].toString();
        if(type == "smartphone") {
            mType = SerialDeviceTypeSmartphone;
        } else if(type == "featurephone") {
            mType = SerialDeviceTypeFeaturePhone;
        } else if(type == "tablet") {
            mType = SerialDeviceTypeTablet;
        } else if(type == "mifi") {
            mType = SerialDeviceTypeMifi;
        } else {
            return false;
        }

        if(obj.contains("id")) {
            mID = obj["id"].toString();
        }

        mMake = obj["make"].toString();
        mModel = obj["model"].toString();

        // This isn't a required parameter.
        if(obj.contains("codename")) {
            mCodename = obj["codename"].toString();
        }

        if(obj.contains("flags")) {
            mFlags = obj["flags"].toInt();
        }

        return true;
    }

    return false;
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

    // This is the amount the progress should increase after each command,
    // that is, the modifier to which we increase the progress so that we
    // reach 100% at completion.
    float mod = (98.0f / (data->constCommands().size() + 3)) / devices.size();

    // The progress starts at 2 (downloading the provision data is 1%, and parsing it is another 1%).
    float i = 2;

    foreach(SerialDevice* device, devices) {
        qDebug()<<"===== Provisioning port"<<device->communicator()->port()<<"=====";

        qDebug()<<"===== SETTING DEVICE OFFLINE BEFORE =====";
        Serial::QCDM::Commands::RadioModeCommand radioCmdBefore(device, Serial::QCDM::MODE_RADIO_OFFLINE);
        radioCmdBefore.execute();

        i += mod;
        emit provisionProgressChanged(SerialProvisionStatusProgress, i);

        if(mProvisionStop) {
            break;
        }

        if(data->password16().size() == 16) {
            qDebug()<<"===== Sending password =====";
            Serial::QCDM::Commands::PasswordCommand passwordCmd(device, data->password16());
            passwordCmd.setTimeout(10000);
            passwordCmd.execute();
            if(passwordCmd.result()->success()) {
                qDebug()<<"Result="<<passwordCmd.result()->data().toString();
            } else {
                qCritical()<<"Could not send password";
                ret = false;
                break;
            }
        }

        if(mProvisionStop) {
            break;
        }

        qDebug()<<"===== Writing SPC =====";
        Serial::QCDM::Commands::QcdmCommand spcCommand(device, Serial::QCDM::DiagCommands::DIAG_SPC_F, data->carrierSPC().toLatin1());
        spcCommand.setTimeout(10000);
        spcCommand.execute();
        if(spcCommand.result()->success()) {
            qDebug()<<"Result="<<spcCommand.result()->data().toString();
        } else {
            qCritical()<<"Could not unlock SPC";
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(SerialProvisionStatusProgress, i);

        if(mProvisionStop) {
            break;
        }

        qDebug()<<"===== WRITING MDN =====";
        Serial::QCDM::Commands::Nv::MDNCommand mdnCmd(device, false, mNewMdn);
        mdnCmd.setTimeout(10000);
        mdnCmd.execute();
        if(mdnCmd.result()->success()) {
            qDebug()<<"Result="<<mdnCmd.result()->data().toString();
        } else {
            qCritical()<<"Could not write MDN";
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(SerialProvisionStatusProgress, i);

        if(mProvisionStop) {
            break;
        }

        qDebug()<<"===== WRITING MIN =====";
        Serial::QCDM::Commands::Nv::MINCommand minCmd(device, false, mNewMin);
        minCmd.setTimeout(10000);
        minCmd.execute();
        if(minCmd.result()->success()) {
            qDebug()<<"Result="<<minCmd.result()->data().toString();
        } else {
            qCritical()<<"Could not write MIN";
            ret = false;
            break;
        }

        i += mod;
        emit provisionProgressChanged(SerialProvisionStatusProgress, i);

        if(mProvisionStop) {
            break;
        }

        qDebug()<<"Provision items"<<data->constCommands().size();

        foreach(Serial::SerialCommand* cmd, data->constCommands()) {
            if(!provision(device, data, cmd)) {
                ret = false;
                break;
            }

            i += mod;
            emit provisionProgressChanged(SerialProvisionStatusProgress, i);

            if(mProvisionStop) {
                break;
            }
        }

        if(!ret || mProvisionStop) {
            break;
        }

        // We need to reset the command results for the next device.
        data->resetCommands();

        qDebug()<<"===== SETTING DEVICE OFFLINE AFTER =====";
        Serial::QCDM::Commands::RadioModeCommand radioCmdAfter(device, Serial::QCDM::MODE_RADIO_OFFLINE);
        radioCmdAfter.execute();

        i += mod;
        emit provisionProgressChanged(SerialProvisionStatusProgress, i);

        if(mProvisionStop) {
            break;
        }
    }

    if(!mProvisionStop) {
        qDebug()<<"Reset device started at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

        qDebug()<<"===== RESETTING DEVICE AFTER =====";
        Serial::QCDM::Commands::RadioModeCommand radioResetAfter(this, Serial::QCDM::MODE_RADIO_RESET);
        radioResetAfter.setTimeout(0);
        radioResetAfter.execute();
        if(!radioResetAfter.resultSuccess()) {
            qWarning()<<"Could not reset device";
        }

        qDebug()<<"Reset device finished at"<<QDateTime::currentDateTime().toString(Qt::ISODate);
    }

    if(ret) {
        emit provisionProgressChanged(SerialProvisionStatusDone, 100);
    } else {
        // If we can't communicate with the device anymore, consider it removed.
        if(!isValid()) {
            emit provisionProgressChanged(SerialProvisionStatusError, i, SerialProvisionErrorRemoved);
        } else {
            emit provisionProgressChanged(SerialProvisionStatusError, i, SerialProvisionErrorNv);
        }
    }

    return ret;
}

bool SerialDevice::provision(SerialDevice* device, SerialProvisionData* data, SerialCommand* cmd) {
    qInfo()<<"Provisioning"<<cmd->debuggingName();

    if(data->sequentialOffline()) {
        if(data->password16().size() == 16) {
            Serial::QCDM::Commands::PasswordCommand passwordCmd(device, data->password16());
            passwordCmd.setTimeout(10000);
            passwordCmd.execute();
            if(!passwordCmd.resultSuccess()) {
                return false;
            }
        }

        Serial::QCDM::Commands::QcdmCommand spcCommand(device, Serial::QCDM::DiagCommands::DIAG_SPC_F, data->carrierSPC().toLatin1());
        spcCommand.setTimeout(10000);
        spcCommand.execute();
        if(!spcCommand.resultSuccess()) {
            return false;
        }
    }

    cmd->execute(device);
    foreach(Serial::SerialCommandResult* result, cmd->results()) {
        if(!result->success()) {
            qDebug()<<"Failed to provision";
            return false;
        }
    }

    if(data->sequentialOffline()) {
        Serial::QCDM::Commands::RadioModeCommand radioCmd(device, Serial::QCDM::MODE_RADIO_OFFLINE);
        radioCmd.setTimeout(10000);
        radioCmd.execute();
        if(!radioCmd.resultSuccess()) {
            return false;
        }
    }

    return true;
}
