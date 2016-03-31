#include <QDebug>

#include "qcdm/commands/nvcommands/nvcommand.h"

#include "serialitemparser.h"

using namespace Serial;

SerialItemParser::SerialItemParser(SerialDevice* device)
    : mDevice(device)
{

}

bool SerialItemParser::checkItem(const QJsonObject &obj) {
    QVariant checkData = 0;
    uint16_t id = 0;

    Serial::SerialCommand *cmd = getItem(obj, false, checkData, id);
    if(cmd == nullptr) {
        return false;
    }

    if(obj.contains("id")) {
        if(mCachedData.contains(id)) {
            return nvDataEquals(checkData, mCachedData[id]);
        }
    }

    if(obj.contains("offset")) {
        cmd->setOffset(obj["offset"].toInt());
    }

    cmd->execute();
    if(!cmd->resultSuccess()) {
        delete cmd;
        return false;
    }

    QVariant outData = cmd->result()->data();

    delete cmd;

    if(obj.contains("id")) {
        mCachedData.insert(obj["id"].toInt(), outData);
    }

    qDebug() << "checkNvItem" << id << checkData << "vs" << outData;
    return nvDataEquals(checkData, outData);
}

Serial::SerialCommand* SerialItemParser::getItem(const QJsonObject& obj) {
    QVariant checkData;
    uint16_t id;
    return getItem(obj, false, checkData, id);
}

Serial::SerialCommand* SerialItemParser::getItem(const QJsonObject &obj, bool isCheck, QVariant& checkData, uint16_t& id) {
    if(!obj.contains("type") || !obj.contains("datatype") || !obj.contains("value")) {
        return nullptr;
    }

    ItemType itemType = getItemType(obj["type"].toString().toLower());
    if(itemType == ItemTypeNone) {
        return nullptr;
    }

    DataType dataType = DataTypeNone;
    checkData = getValue(obj["datatype"].toString().toLower(), obj["value"].toString(), dataType);

    if(dataType == DataTypeNone) {
        return nullptr;
    }

    id = 0;
    if(obj.contains("id")) {
        id = obj["id"].toInt();
    }

    return getItem(itemType, dataType, ((isCheck)?0:checkData), id);
}

bool SerialItemParser::nvDataEquals(const QVariant& checkData, const QVariant& nvData) {
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

SerialItemParser::ItemType SerialItemParser::getItemType(const QString& itemType) {
    if(itemType == "at") {
        return ItemTypeAT;
    } else if(itemType == "qcdm") {
        return ItemTypeQCDM;
    } else if(itemType == "nv") {
        return ItemTypeNV;
    }

    return ItemTypeNone;
}

Serial::SerialCommand* SerialItemParser::getItem(ItemType itemType, DataType dataType, QVariant checkData, uint16_t id) {
    Serial::SerialCommand* cmd = nullptr;
    switch(itemType) {
    case ItemTypeAT:
        break;
    case ItemTypeQCDM:
        break;
    case ItemTypeNV:
        switch(dataType) {
            case DataType8Bit: {
                cmd = new Serial::QCDM::Commands::Nv::NvCommand8Bit(mDevice, true, static_cast<Serial::QCDM::NvItem>(id), checkData.toUInt());
                break;
            }
            case DataType16Bit: {
                cmd = new Serial::QCDM::Commands::Nv::NvCommand16Bit(mDevice, true, static_cast<Serial::QCDM::NvItem>(id), checkData.toUInt());
                break;
            }
            case DataType32Bit: {
                cmd = new Serial::QCDM::Commands::Nv::NvCommand32Bit(mDevice, true, static_cast<Serial::QCDM::NvItem>(id), checkData.toUInt());
                break;
            }
            case DataType64Bit: {
                cmd = new Serial::QCDM::Commands::Nv::NvCommand64Bit(mDevice, true, static_cast<Serial::QCDM::NvItem>(id), checkData.toULongLong());
                break;
            }
            case DataTypeString: {
                cmd = new Serial::QCDM::Commands::Nv::NvCommandString(mDevice, true, static_cast<Serial::QCDM::NvItem>(id), checkData.toString());
                break;
            }
            case DataTypeRaw: {
                cmd = new Serial::QCDM::Commands::Nv::NvCommand(mDevice, true, static_cast<Serial::QCDM::NvItem>(id), checkData.toByteArray());
            }
            default: {
                break;
            }
        }

        break;
    default:
        break;
    }

    return cmd;
}

QVariant SerialItemParser::getValue(const QString& type, const QString& data, DataType& outType) {
    if(type == "8bit") {
        outType = DataType8Bit;
        return static_cast<uint8_t>(data.toUShort(0, 0));
    } else if(type == "16bit") {
        outType = DataType16Bit;
        return static_cast<uint16_t>(data.toUShort(0, 0));
    } else if(type == "32bit") {
        outType = DataType32Bit;
        return static_cast<uint32_t>(data.toUInt(0, 0));
    } else if(type == "64bit") {
        outType = DataType64Bit;
        return static_cast<uint64_t>(data.toULongLong(0, 0));
    } else if(type == "string") {
        outType = DataTypeString;
        return data;
    } else if(type == "raw") {
        outType = DataTypeRaw;
        return QByteArray::fromHex(data.toLatin1());
    }

    return QVariant();
}
