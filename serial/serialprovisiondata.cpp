#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QUrl>

#include "qcdm/commands/qcdmcommand.h"
#include "serialprovisiondata.h"

using namespace Serial;

SerialProvisionData::SerialProvisionData(SerialDevice* device)
    : mDevice(device)
{
    mSequentialOffline = false;
    mPassword16 = "";
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
    mCarrierSPC = rootObject["carrierSPC"].toString();
    if(rootObject.contains("sequentialOffline")) {
        mSequentialOffline = rootObject["sequentialOffline"].toString().toUInt() != 0;
    }
    if(rootObject.contains("sixteendigitpassword") && rootObject["sixteendigitpassword"].toString() != "null") {
        mPassword16 = rootObject["sixteendigitpassword"].toString();
    }
    mUserType = static_cast<UserType>(rootObject["userType"].toString().toUInt());
    mUser = getUser(mDevice, mUserType, rootObject["user"].toString());
    mUserProfIndex = 0;
    if(rootObject.contains("genUserProf")) {
        mUserProfIndex = static_cast<uint8_t>(rootObject["genUserProf"].toObject()["index"].toString().toUInt());
    }

    updateObj("", rootObject);

    if(rootObject.contains("calibrationFile") && rootObject["calibrationFile"].toString() != "null") {
        QString md5 = "";
        if(rootObject.contains("calibrationFilemd5")) {
            md5 = rootObject["calibrationFilemd5"].toString();
        }
        updateCalibration(QUrl(rootObject["calibrationFile"].toString()), md5);
    }
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

        mCommands.append(cmd);
    }
}

QString SerialProvisionData::getUser(SerialDevice* device, UserType type, const QString& userNai) {
    switch(type) {
    case UserTypeESN:
        return device->esnStr() + userNai;
    case UserTypeMIN:
        return device->minStr() + userNai;
    case UserTypeMEID:
        return device->meidStr() + userNai;
    default:
        return device->mdnStr() + userNai;
    }
}

QString SerialProvisionData::getPassword(SerialDevice* device, const QString& str) {
    QString strLow = str.toLower();
    if (strLow == "meid") {
        return device->meidStr();
    } else if (strLow == "decesn") {
        return QString("%1").arg(device->esn(), 10, 10, QChar('0'));
    } else if (strLow == "min") {
        return device->minStr();
    } else if (strLow == "esn") {
        return device->esnStr();
    }

    return str;
}
