#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QUrl>

#include "qcdm/commands/qcdmcommand.h"
#include "serialprovisiondata.h"

using namespace Serial;

SerialProvisionData::SerialProvisionData(SerialDevice* device)
    : mDevice(device),
      mValid(false),
      mRTREMode(Serial::QCDM::RTREModeNVOnly),
      mCarrierSPC(""),
      mSequentialOffline(false),
      mPassword16(""),
      mUserType(UserTypeNone),
      mUser(""),
      mUserProfIndex(0)
{

}

SerialProvisionData::~SerialProvisionData() {
    foreach(SerialCommand* cmd, mCommands) {
        delete cmd;
    }
}

void SerialProvisionData::resetCommands() {
    foreach(SerialCommand* cmd, mCommands) {
        cmd->clearResults();
    }
}

void SerialProvisionData::update(const QJsonObject& rootObject) {
    if(rootObject.contains("rtreMode")) {
        mRTREMode = static_cast<Serial::QCDM::RTREMode>(rootObject["rtreMode"].toInt());
    }
    if(rootObject.contains("carrierSPC")) {
        mCarrierSPC = rootObject["carrierSPC"].toString();
    }
    if(rootObject.contains("sequentialOffline")) {
        mSequentialOffline = rootObject["sequentialOffline"].toString().toUInt() != 0;
    }
    if(rootObject.contains("sixteendigitpassword") && rootObject["sixteendigitpassword"].toString().length() > 0) {
        mPassword16 = rootObject["sixteendigitpassword"].toString();
    }
    if(rootObject.contains("userType")) {
        mUserType = static_cast<UserType>(rootObject["userType"].toString().toUInt());
    }
    if(rootObject.contains("user")) {
        mUser = getUser(mDevice, mUserType, rootObject["user"].toString());
    }
    if(rootObject.contains("genUserProf")) {
        mUserProfIndex = static_cast<uint8_t>(rootObject["genUserProf"].toObject()["index"].toString().toUInt());
    }

    updateObj("", rootObject);

    if(rootObject.contains("calibrationFile") && rootObject["calibrationFile"].toString().length() > 0) {
        QString md5 = "";
        if(rootObject.contains("calibrationFilemd5") && rootObject["calibrationFilemd5"].toString().length() > 0) {
            md5 = rootObject["calibrationFilemd5"].toString();
        }
        updateCalibration(QUrl(rootObject["calibrationFile"].toString()), md5);
    }

    mValid = true;
}

void SerialProvisionData::updateObj(const QString& parent, const QJsonObject& obj) {
    for(QJsonObject::const_iterator i = obj.constBegin(); i != obj.constEnd(); ++i) {
        SerialCommand* cmd = getCommand(parent, i.key(), i.value());
        if(cmd == nullptr) {
            if(i.value().isObject()) {
                QString p = parent + ((parent.size() == 0)?"":"/") + i.key();
                updateObj(p, i.value().toObject());
            }

            continue;
        }

        cmd->setTimeout(10000);
        mCommands.append(cmd);
    }
}

QString SerialProvisionData::getUser(SerialDevice* device, UserType type, const QString& userNai) {
    switch(type) {
    case UserTypeESN:
        return device->esnStr() + userNai;
    case UserTypeMIN:
        return device->newMinStr() + userNai;
    case UserTypeMEID:
        return device->meidStr() + userNai;
    default:
        return device->newMdnStr() + userNai;
    }
}

QString SerialProvisionData::getPassword(SerialDevice* device, const QString& str) {
    QString strLow = str.toLower();
    if (strLow == "meid") {
        return device->meidStr();
    } else if (strLow == "decesn") {
        return QString("%1").arg(device->esn(), 10, 10, QChar('0'));
    } else if (strLow == "min") {
        return device->newMinStr();
    } else if (strLow == "esn") {
        return device->esnStr();
    }

    return str;
}
