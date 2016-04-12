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

SerialDeviceConfig::SerialDeviceConfig()
{
    QFile targetFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.bin");
    if(!targetFile.open(QFile::ReadOnly)) {
        return;
    }

    mDevices = QJsonDocument::fromBinaryData(targetFile.readAll());
    if(!mDevices.isObject()) {
        return;
    }
}

SerialDeviceConfig::~SerialDeviceConfig() {

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
            foundDevice = device->updateJson(obj);
            break;
        }
     }

     return foundDevice;
}

void SerialDeviceConfig::updateConfig() {
    QFile sourceFile(QStringLiteral(":/res/data/devices.list"));
    QFile targetFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data/devices.bin");
    if(targetFile.exists()) {
        if(QFileInfo(sourceFile).size() != QFileInfo(targetFile).size()) {
            targetFile.remove();
        }
    }

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
