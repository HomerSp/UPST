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
#include "qcdm/commands/nvcommands/rtrecommand.h"
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
      mGuide(QJsonObject()),
      mMdn(""),
      mMin(-1),
      mESN(0),
      mIMEI(0),
      mMEID(0),
      mRTRE(Serial::QCDM::RTREModeNone),
      mProvisioning(false),
      mProvisionStop(false),
      mRTRESet(false),
      mNewMdn(""),
      mNewMin(-1),
      mSPC(""),
      mWrongSPC(false)
{
    if(!port.isEmpty()) {
        mCommunicator = new SerialCommunicator(*this);
        if(mCommunicator->open()) {
            mCommunicator->clear();
        } else {
            delete mCommunicator;
            mCommunicator = nullptr;
        }
    }
}

SerialDevice::SerialDevice(const QSerialPortInfo& info)
    : SerialDevice(info.portName(), info.vendorIdentifier(), info.productIdentifier()) {

}

SerialDevice::SerialDevice(const QJsonObject& obj)
    : SerialDevice("", 0, 0)
{
    updateJson(obj);
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

bool SerialDevice::close() {
    bool ret = true;

    if(mCommunicator != nullptr) {
        ret = mCommunicator->close();
        delete mCommunicator;
    }
    mCommunicator = nullptr;

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
    foreach(Serial::SerialDevice* c, mChildren) {
        if(!c->isAvailable()) {
            return false;
        }
    }

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

bool SerialDevice::childrenAvailable() {
    if(mChildren.size() <= 0) {
        return false;
    }

    bool ret = true;
    foreach(Serial::SerialDevice* d, mChildren) {
        if(!d->isAvailable()) {
            ret = false;
            break;
        }
    }

    return ret;
}

bool SerialDevice::canProvision() {
    if(isAvailable() && mType != Serial::SerialDeviceTypeUnknown && !isProvisioning()) {
        if(!flagMultiPort()) {
            return true;
        }

        return (childrenAvailable());
    }

    return false;
}

void SerialDevice::handleEventReport(const QByteArray& data) {
    Q_UNUSED(data);
    qWarning()<<"Ignoring event report message";
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

bool SerialDevice::update() {
    qDebug()<<"===== GETTING ESN =====";
    Serial::QCDM::Commands::Nv::ESNCommand esnCmd(this);
    esnCmd.execute();
    if(esnCmd.result()->success()) {
        mESN = esnCmd.result()->data().toUInt();
    } else {
        return false;
    }

    qDebug()<<"===== GETTING IMEI =====";
    Serial::QCDM::Commands::Nv::IMEICommand imeiCmd(this);
    imeiCmd.execute();
    if(imeiCmd.result()->success()) {
        mIMEI = imeiCmd.result()->data().toULongLong();
    } else if(imeiCmd.result()->timedOut()) {
        return false;
    }

    qDebug()<<"===== GETTING MEID =====";
    Serial::QCDM::Commands::Nv::MEIDCommand meidCmd(this);
    meidCmd.execute();
    if(meidCmd.result()->success()) {
        mMEID = meidCmd.result()->data().toULongLong();
    } else if(meidCmd.result()->timedOut()) {
        return false;
    }

    qDebug()<<"===== GETTING MDN =====";
    Serial::QCDM::Commands::Nv::MDNCommand mdnCmd(this);
    mdnCmd.execute();
    if(mdnCmd.result()->success()) {
        mMdn = mdnCmd.result()->data().toString();
    } else if(mdnCmd.result()->timedOut()) {
        return false;
    }

    qDebug()<<"===== GETTING MIN =====";
    Serial::QCDM::Commands::Nv::MINCommand minCmd(this);
    minCmd.execute();
    if(minCmd.result()->success()) {
        mMin = minCmd.result()->data().toULongLong();
    } else if(minCmd.result()->timedOut()) {
        return false;
    }

    qDebug()<<"====== GETTING RTRE ======";
    Serial::QCDM::Commands::Nv::RTRECommand rtreCommand(this);
    rtreCommand.execute();
    if(rtreCommand.result()->success()) {
        mRTRE = rtreCommand.mode();
    } else if(rtreCommand.result()->timedOut()) {
        return false;
    }

    if(mMin == static_cast<uint64_t>(-1)) {
        QString number = QString("%1").arg(mMEID, 4, 10, QChar('0'));
        number = number.mid(number.length() - 4, 4);
        mNewMin = number.toULongLong();
    }
    if(mMdn.length() == 0) {
        QString number = QString("%1").arg(mMEID, 4, 10, QChar('0'));
        number = number.mid(number.length() - 4, 4);
        mNewMdn = "000000" + number;
    }

    qDebug()<<"ESN:"<<QString::number(mESN, 16)<<"IMEI:"<<QString::number(mIMEI, 16)<<"MEID:"<<QString::number(mMEID, 16)<<"MDN:"<<mMdn<<", MIN:"<<mMin<<", RTRE:"<<mRTRE;

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

        if(obj.contains("guide")) {
            mGuide = obj["guide"].toObject();
        }

        return true;
    }

    return false;
}

void SerialDevice::updateFrom(SerialDevice *device, Serial::SerialDevice* other) {
    device->mCommunicator = other->mCommunicator;
    other->mCommunicator = nullptr;

    device->mRTRE = other->mRTRE;

    // Don't update the min and mdn if we have set the RTRE.
    if(!device->mRTRESet) {
        device->mMdn = other->mMdn;
        device->mMin = other->mMin;
        device->mNewMdn = other->mMdn;
        device->mNewMin = other->mMin;

        if(other->mMin == static_cast<uint64_t>(-1)) {
            device->mNewMdn = "0000000000";
            device->mNewMin = 0;
        }

    }
}

void SerialDevice::updateFrom(Serial::SerialDevice* other) {
    QList<Serial::SerialDevice*> devices;
    devices.append(this);
    devices.append(mChildren);

    // Check if the port matches first.
    foreach(Serial::SerialDevice* d, devices) {
        if(d->port() == other->port() && !d->isAvailable()) {
            updateFrom(d, other);
            return;
        }
    }

    // If it doesn't, we check that the make and model match, and that the device is not available.
    foreach(Serial::SerialDevice* d, devices) {
        if(d->mModel == other->mModel && d->mMake == other->mMake && !d->isAvailable()) {
            updateFrom(d, other);
            return;
        }
    }
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

    bool shouldSetRtre = false;
    foreach(SerialDevice* device, devices) {
        if(!device->mRTRESet && device->mRTRE != data->rtreMode()) {
            shouldSetRtre = true;
            break;
        }
    }

    if(mSPC.size() == 0) {
        mSPC = data->carrierSPC();
    }

    mWrongSPC = false;

    foreach(SerialDevice* device, devices) {
        qDebug()<<"===== Provisioning port"<<mPort<<"=====";

        // If we don't have an SPC, break here before any of the actual provisioning.
        if(mProvisionStop || mSPC.length() == 0) {
            break;
        }

        if(shouldSetRtre) {
            if(data->password16().size() == 16) {
                for(int i = 0; i < 5; i++) {
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
            }

            if(mProvisionStop) {
                break;
            }

            qDebug()<<"===== Writing SPC =====";
            if(!sendSPC(device)) {
                qCritical()<<"Could not unlock SPC";
                mWrongSPC = true;
                ret = false;
                break;
            }

            if(mProvisionStop) {
                break;
            }

            qDebug()<<"===== SETTING RTRE MODE =====";

            device->mRTRESet = true;

            Serial::QCDM::Commands::Nv::RTRECommand rtreCommand(device, false, data->rtreMode());
            rtreCommand.execute();
            continue;
        }

        qDebug()<<"===== SETTING DEVICE OFFLINE BEFORE =====";
        Serial::QCDM::Commands::RadioModeCommand radioCmdBefore(device, Serial::QCDM::MODE_RADIO_OFFLINE);
        radioCmdBefore.execute();

        if(mProvisionStop) {
            break;
        }

        i += mod;
        emit provisionProgressChanged(SerialProvisionStatusProgress, i);

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
        if(!sendSPC(device)) {
            qCritical()<<"Could not unlock SPC";
            mWrongSPC = true;
            ret = false;
            break;
        }

        if(mProvisionStop) {
            break;
        }

        if(shouldSetRtre) {
            qDebug()<<"===== SETTING RTRE MODE =====";

            device->mRTRESet = true;

            Serial::QCDM::Commands::Nv::RTRECommand rtreCommand(device, false, data->rtreMode());
            rtreCommand.execute();
            continue;
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
            qCritical()<<"Could not write MDN"<<mdnCmd.result()->errorCode();
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
            qCritical()<<"Could not write MIN"<<minCmd.result()->errorCode();
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
                qInfo()<<"Provisioning failed";
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

    // We need to check if the device is valid *before* the reset.
    bool valid = isValid();

    if(!mProvisionStop && !mWrongSPC && mSPC.length() > 0) {
        qDebug()<<"Reset device started at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

        qDebug()<<"===== RESETTING DEVICE AFTER =====";
        Serial::QCDM::Commands::RadioModeCommand radioResetAfter(this, Serial::QCDM::MODE_RADIO_RESET);
        radioResetAfter.setTimeout((flagManualReboot())?1000:0);
        radioResetAfter.execute();
        if(!radioResetAfter.resultSuccess()) {
            qWarning()<<"Could not reset device";
        }

        qDebug()<<"Reset device finished at"<<QDateTime::currentDateTime().toString(Qt::ISODate);
    }

    if(ret) {
        emit provisionProgressChanged((shouldSetRtre)?SerialProvisionStatusDoneRTRE:SerialProvisionStatusDone, 100);
    } else {
        if(mRTRESet && mRTRE != data->rtreMode()) {
            qCritical()<<"Wrong RTRE mode, device has"<<mRTRE<<"while provision data has"<<data->rtreMode();
            emit provisionProgressChanged(SerialProvisionStatusError, i, SerialProvisionErrorRTRE);
        } else if(mWrongSPC) {
            emit provisionProgressChanged(SerialProvisionStatusWrongSPC, 100);

        // If we can't communicate with the device anymore, consider it removed.
        } else if(!valid) {
            emit provisionProgressChanged(SerialProvisionStatusError, i, SerialProvisionErrorRemoved);
        } else {
            emit provisionProgressChanged(SerialProvisionStatusError, i, SerialProvisionErrorNv);
        }
    }

    if(!shouldSetRtre) {
        foreach(SerialDevice* device, devices) {
            device->mRTRESet = false;
        }
    }

    return ret;
}

bool SerialDevice::provision(SerialDevice* device, SerialProvisionData* data, SerialCommand* cmd) {
    qInfo()<<"Provisioning"<<cmd->name();

    if(data->sequentialOffline()) {
        if(data->password16().size() == 16) {
            Serial::QCDM::Commands::PasswordCommand passwordCmd(device, data->password16());
            passwordCmd.setTimeout(10000);
            passwordCmd.execute();
            if(!passwordCmd.resultSuccess()) {
                return false;
            }
        }

        if(!sendSPC(device)) {
            qCritical()<<"Could not unlock SPC";
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

bool SerialDevice::sendSPC(SerialDevice* device) {
    Serial::QCDM::Commands::QcdmCommand spcCommand(device, Serial::QCDM::DiagCommands::DIAG_SPC_F, mSPC.toLatin1());
    spcCommand.setTimeout(10000);
    spcCommand.execute();
    return (spcCommand.resultSuccess() && (spcCommand.result()->data().toByteArray().at(0) == 0x01));
}

QString SerialDevice::portStr() const {
    if(mChildren.size() == 0) {
        return mPort;
    }

    QString ret = mPort;
    foreach(SerialDevice* d, mChildren) {
        ret += ", " + d->port();
    }

    return ret;
}
QString SerialDevice::vidStr() const {
    return QString("%1").arg(mVid, 4, 16, QChar('0')).toUpper();
}
QString SerialDevice::pidStr() const {
    return QString("%1").arg(mPid, 4, 16, QChar('0')).toUpper();
}
QString SerialDevice::name() const {
    QString ret = "";
    if(mMake.length() != 0) {
        ret += mMake;
    }
    if(mModel.length() != 0) {
        if(ret.length() > 0) {
            ret += " ";
        }

        ret += mModel;
    }

    return ret;
}
QString SerialDevice::makeStr() const {
    if(mMake.length() == 0) {
        return "-";
    }

    return mMake;
}
QString SerialDevice::modelStr() const {
    if(mModel.length() == 0) {
        return "-";
    }

    return mModel;
}
QString SerialDevice::mdnStr() const {
    if(mMdn.length() == 0) {
        return "-";
    }

    return mMdn;
}
QString SerialDevice::minStr() const {
    if(mMin == static_cast<uint64_t>(-1)) {
        return "-";
    }

    return QString("%1").arg(mMin, 10, 10, QChar('0'));
}
QString SerialDevice::esnStr() const {
    if(mESN == 0) {
        return "-";
    }

    return QString("%1").arg(mESN, 8, 16, QChar('0')).toUpper();
}
QString SerialDevice::meidStr() const {
    if(mMEID == 0) {
        return "-";
    }

    return QString("%1").arg(mMEID, 14, 16, QChar('0')).toUpper();
}
QString SerialDevice::imeiStr() const {
    if(mIMEI == 0) {
        return "-";
    }

    return QString("%1").arg(mIMEI, 14, 16, QChar('0')).toUpper();
}
QString SerialDevice::rtreStr() const {
    switch(mRTRE) {
    case Serial::QCDM::RTREModeRUIMOnly:
        return "RUIM Only";
    case Serial::QCDM::RTREModeNVOnly:
        return "NV Only";
    case Serial::QCDM::RTREModeRUIMPref:
        return "RUIM Pref";
    case Serial::QCDM::RTREMode1XGSM:
        return "1xGSM";
    default:
        return "-";
    }
}
QString SerialDevice::newMdnStr() const {
    if(mNewMdn.length() == 0) {
        return mMdn;
    }

    return mNewMdn;
}
QString SerialDevice::newMinStr() const {
    if(mNewMin == static_cast<uint64_t>(-1)) {
        QString ret = minStr();
        if(ret == "-") {
            return "";
        }

        return ret;
    }

    return QString("%1").arg(mNewMin, 10, 10, QChar('0'));
}
