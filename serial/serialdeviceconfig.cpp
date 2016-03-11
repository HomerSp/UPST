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

     QHash<int, QVariant> nvItems;
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

        bool isCorrect = true;

        if(obj.contains("nv")) {
            QJsonArray nvArr = obj["nv"].toArray();
            foreach(QJsonValue nvVal, nvArr) {
                QJsonObject nvObj = nvVal.toObject();
                if(!nvObj.contains("id") || !nvObj.contains("type") || !nvObj.contains("value")) {
                    isCorrect = false;
                    break;
                }

                uint32_t id = nvObj["id"].toInt();
                NvType type = NvTypeNone;
                QVariant checkData = getValue(nvObj["type"].toString(), nvObj["value"].toString(), type);
                if(type == NvTypeNone) {
                    isCorrect = false;
                    break;
                }

                if(nvItems.contains(id)) {
                    isCorrect = nvDataEquals(checkData, nvItems[id]);
                    break;
                }

                QVariant outData;
                if(!checkNvItem(device, id, type, checkData, outData)) {
                    isCorrect = false;
                }

                if(!outData.isNull()) {
                    nvItems.insert(id, outData);
                }
            }
        }

        if(isCorrect) {
            device->updateJson(obj);
            break;
        }
     }

     return true;
}

bool SerialDeviceConfig::checkNvItem(SerialDevice* device, uint16_t id, NvType type, const QVariant& checkData, QVariant& outData) {
    Serial::QCDM::Commands::QcdmCommand *cmd = nullptr;
    switch(type) {
        case NvType8Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand8Bit(device, true, static_cast<Serial::QCDM::NvItem>(id));
            break;
        }
        case NvType16Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand16Bit(device, true, static_cast<Serial::QCDM::NvItem>(id));
            break;
        }
        case NvType32Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand32Bit(device, true, static_cast<Serial::QCDM::NvItem>(id));
            break;
        }
        case NvType64Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand64Bit(device, true, static_cast<Serial::QCDM::NvItem>(id));
            break;
        }
        case NvTypeString: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommandString(device, true, static_cast<Serial::QCDM::NvItem>(id));
            break;
        }
        default: {
            break;
        }
    }

    if(cmd == nullptr) {
        return false;
    }

    cmd->execute();
    if(!cmd->resultSuccess()) {
        delete cmd;
        return false;
    }

    outData = cmd->result()->data();

    delete cmd;

    qDebug() << "checkNvItem" << id << checkData << "vs" << outData;
    return nvDataEquals(checkData, outData);
}

 bool SerialDeviceConfig::nvDataEquals(const QVariant& checkData, const QVariant& nvData) {
     if(checkData.type() == QVariant::String && nvData.type() == checkData.type()) {
        QString checkString = checkData.toString().toLower();
        QString nvString = nvData.toString().toLower();

        if(checkString.contains('*')) {
            if(checkString.startsWith('*') && checkString.endsWith('*')) {
                return nvString.contains(checkString.mid(1, checkString.length() - 2));
             } else if(checkString.startsWith('*')) {
                return nvString.endsWith(checkString.mid(1));
            } else if(checkString.endsWith('*')) {
                return nvString.startsWith(checkString.mid(0, checkString.length() - 1));
            }
        }

        return nvString == checkString;
     }

     return checkData == nvData;
 }

QVariant SerialDeviceConfig::getValue(const QString& type, const QString& data, NvType& outType) {
    if(type == "8bit") {
        outType = NvType8Bit;
        return data.toShort(0, 0);
    } else if(type == "16bit") {
        outType = NvType16Bit;
        return data.toShort(0, 0);
    } else if(type == "32bit") {
        outType = NvType32Bit;
        return data.toInt(0, 0);
    } else if(type == "64bit") {
        outType = NvType64Bit;
        return data.toLongLong(0, 0);
    } else if(type == "string") {
        outType = NvTypeString;
        return data;
    }

    return QVariant();
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
