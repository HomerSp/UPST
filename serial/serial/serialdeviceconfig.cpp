#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "qcdm/commands/nvcommands/nvcommand.h"
#include "serialdeviceconfig.h"
#include "serialitemparser.h"

using namespace Serial;

SerialDeviceConfig::SerialDeviceConfig(const QString& data)
    : mIsValid(false)
{
    mIsValid = update(data);
    if(!mIsValid) {
        QFile targetFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.bin");
        if(!targetFile.open(QFile::ReadOnly)) {
            return;
        }

        mDevices = QJsonDocument::fromBinaryData(targetFile.readAll());
        mIsValid = mDevices.isObject();
    }
}

SerialDeviceConfig::~SerialDeviceConfig() {

}

QList<SerialDevice*> SerialDeviceConfig::getDeviceGuides() {
    QList<SerialDevice*> ret;

    if(!mDevices.isObject() || mDevices.object().isEmpty() || !mDevices.object().contains("devices")) {
        return ret;
    }

     QJsonArray devicesArr = mDevices.object()["devices"].toArray();
     if(devicesArr.isEmpty()) {
         return ret;
     }

     foreach(QJsonValue val, devicesArr) {
        QJsonObject obj = val.toObject();
        if(obj.contains("guide")) {
            ret.append(new SerialDevice(obj));
        }
     }

     return ret;
}

bool SerialDeviceConfig::update(const QString& data) {
#ifdef TESTING_MODE
    if(QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.json").exists()) {
        return true;
    }
#endif

    QFile targetFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.bin");
    if(targetFile.exists()) {
        targetFile.remove();
    }

    if(!targetFile.exists()) {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data");

        if(!targetFile.open(QIODevice::WriteOnly)) {
            qDebug()<<"!targetFile open";
            return false;
        }

        targetFile.write(QJsonDocument::fromJson(data.toLatin1()).toBinaryData());
        targetFile.flush();
        targetFile.close();
    }

    mDevices = QJsonDocument::fromJson(data.toLatin1());
    return mDevices.isObject();
}

bool SerialDeviceConfig::shouldReschedule(SerialDevice* device) {
    if(!mDevices.isObject() || mDevices.object().isEmpty() || !mDevices.object().contains("devices")) {
        return false;
    }

     QJsonArray devicesArr = mDevices.object()["devices"].toArray();
     if(devicesArr.isEmpty()) {
         return false;
     }

     bool foundDevice = false;
     foreach(QJsonValue val, devicesArr) {
        QJsonObject obj = val.toObject();
        if(!obj.contains("vid") || !obj.contains("pid")) {
            continue;
        }

        uint16_t vid = obj["vid"].toString().toUShort(0, 0);
        if(device->vid() != vid) {
            continue;
        }

        bool pidFound = false;

        QStringList pidList = obj["pid"].toString().split(',');
        foreach(QString pid, pidList) {
            if(pid.toUShort(0, 0) == device->pid()) {
                pidFound = true;
                break;
            }
        }

        if(!pidFound) {
            continue;
        }

        foundDevice = true;
        break;
     }

     return foundDevice;
}

bool SerialDeviceConfig::updateDevice(SerialDevice *device) {
    if(!mDevices.isObject() || mDevices.object().isEmpty() || !mDevices.object().contains("devices")) {
        return false;
    }

     QJsonArray devicesArr = mDevices.object()["devices"].toArray();
     if(devicesArr.isEmpty()) {
         return false;
     }

     SerialItemParser parser(device);

     bool foundDevice = false;
     foreach(QJsonValue val, devicesArr) {
        QJsonObject obj = val.toObject();
        if(!obj.contains("vid") || !obj.contains("pid")) {
            continue;
        }

        uint16_t vid = obj["vid"].toString().toUShort(0, 0);
        if(device->vid() != vid) {
            continue;
        }

        bool pidFound = false;

        QStringList pidList = obj["pid"].toString().split(',');
        foreach(QString pid, pidList) {
            if(pid.toUShort(0, 0) == device->pid()) {
                pidFound = true;
                break;
            }
        }

        if(!pidFound) {
            continue;
        }

        qInfo()<<"Vid and Pid of"<<obj["make"].toString()<<obj["model"].toString()<<"matches, checking items...";

        bool isCorrect = true;

        if(obj.contains("match_items")) {
            QJsonArray matchArr = obj["match_items"].toArray();
            foreach(QJsonValue matchVal, matchArr) {
                QJsonObject nvObj = matchVal.toObject();
                isCorrect = parser.checkItem(nvObj);
                if(!isCorrect) {
                    break;
                }
            }
        }

        if(isCorrect) {
            qInfo()<<"Device"<<obj["make"].toString()<<obj["model"].toString()<<"is a match!";

            foundDevice = device->updateJson(obj);
            break;
        }
     }

     return foundDevice;
}

void SerialDeviceConfig::updateConfig() {
    QFile targetFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.bin");
    if(targetFile.exists()) {
        return;
    }

    QFile sourceFile(QStringLiteral(":/res/data/devices.list"));
#ifdef TESTING_MODE
    if(QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.json").exists()) {
        sourceFile.setFileName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.json");

        targetFile.remove();
    }
#endif

    if(!targetFile.exists()) {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data");

        if(!sourceFile.open(QIODevice::ReadOnly) || !targetFile.open(QIODevice::WriteOnly)) {
            qDebug()<<"!sourceFile open || !targetFile open";
            return;
        }

        targetFile.write(QJsonDocument::fromJson(sourceFile.readAll()).toBinaryData());
        targetFile.flush();
        targetFile.close();

        sourceFile.close();
    }

}
