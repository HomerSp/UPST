#include <QDebug>
#include <QFile>
#include <QJsonDocument>

#include "qcdm/commands/qcdmcommand.h"
#include "serialprovisiondata.h"

using namespace Serial;

SerialProvisionData::SerialProvisionData(SerialDevice* device)
    : mDevice(device)
{

}

SerialProvisionData::~SerialProvisionData() {
    foreach(SerialCommand* cmd, mCommands) {
        delete cmd;
    }
}

void SerialProvisionData::update(const QString &data) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toLatin1());

    QJsonObject rootObject = jsonDoc.object();
    mCarrierSPC = rootObject["carrierSPC"].toString();
    mUserType = static_cast<UserType>(rootObject["userType"].toString().toUInt());
    mUser = getUser(mDevice, mUserType, rootObject["user"].toString());
    mUserProfIndex = 0;
    if(rootObject.contains("genUserProf")) {
        mUserProfIndex = static_cast<uint8_t>(rootObject["genUserProf"].toObject()["index"].toString().toUInt());
    }

    updateObj("", rootObject);
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
