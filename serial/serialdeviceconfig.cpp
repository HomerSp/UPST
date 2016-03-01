#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "serialdeviceconfig.h"

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

     foreach(QJsonValue val, devicesArr) {
        QJsonObject obj = val.toObject();
        if(!obj.contains("vid") || !obj.contains("pid")) {
            continue;
        }

        uint16_t vid = obj["vid"].toString().toUShort(0, 16);
        uint16_t pid = obj["pid"].toString().toUShort(0, 16);
        if(device->vid() != vid || device->pid() != pid) {
            continue;
        }

        device->updateJson(obj);
     }

     return true;
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
