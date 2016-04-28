#include <QDebug>

#include "qcdm/commands/nvcommands/nvcommand.h"

#include "serialitemparser.h"

using namespace Serial;

ItemParser::DataItem::DataItem(const QJsonObject& obj)
    : mItemType(ItemTypeNone),
      mOffset(0),
      mValueType(ValueTypeNone),
      mValue(0)
{
    if(!obj.contains("type")) {
        return;
    }

    mItemType = getItemType(obj["type"].toString());
    if(mItemType == ItemTypeNone) {
        return;
    }

    if(obj.contains("offset")) {
        mOffset = obj["offset"].toInt();
    }
    if(obj.contains("valuetype")) {
        mValueType = getValueType(obj["valuetype"].toString());
    }
    if(obj.contains("value")) {
        mValue = getValue(obj["value"].toString(), mValueType);
    }
}

bool ItemParser::DataItem::equals(const QVariant &value) {
    qDebug()<<"DataItem equals"<<mValue<<"vs"<<value;

    if(mValue.type() == QVariant::String && value.type() == QVariant::String) {
        QString checkString = mValue.toString().toLower();
        QString otherString = value.toString().toLower();

        if(checkString.contains('*')) {
            if(checkString.startsWith('*') && checkString.endsWith('*')) {
                return otherString.contains(checkString.mid(1, checkString.length() - 2));
            } else if(checkString.startsWith('*')) {
                return otherString.endsWith(checkString.mid(1));
            } else if(checkString.endsWith('*')) {
                return otherString.startsWith(checkString.mid(0, checkString.length() - 1));
            }
        }

        return otherString == checkString;
    }

    return mValue == value;
}

ItemParser::ItemType ItemParser::DataItem::getItemType(const QString &type) {
    if(type == "at") {
        return ItemTypeAT;
    } else if(type == "qcdm") {
        return ItemTypeQCDM;
    } else if(type == "nv") {
        return ItemTypeNV;
    }

    return ItemTypeNone;
}

ItemParser::ValueType ItemParser::DataItem::getValueType(const QString &type) const {
    if(type == "8bit") {
        return ItemParser::ValueType8Bit;
    } else if(type == "16bit") {
        return ItemParser::ValueType16Bit;
    } else if(type == "32bit") {
        return ItemParser::ValueType32Bit;
    } else if(type == "64bit") {
        return ItemParser::ValueType64Bit;
    } else if(type == "string") {
        return ItemParser::ValueTypeString;
    } else if(type == "raw") {
        return ItemParser::ValueTypeRaw;
    }

    return ItemParser::ValueTypeNone;
}

QVariant ItemParser::DataItem::getValue(const QString &value, ValueType type) const {
    switch(type) {
    case ValueType8Bit:
        return static_cast<uint8_t>(value.toUShort(0, 0));
    case ValueType16Bit:
        return static_cast<uint16_t>(value.toUShort(0, 0));
    case ValueType32Bit:
        return static_cast<uint32_t>(value.toUInt(0, 0));
    case ValueType64Bit:
        return static_cast<uint64_t>(value.toULongLong(0, 0));
    case ValueTypeString:
        return value;
    case ValueTypeRaw:
        return QByteArray::fromHex(value.toLatin1());
    default:
        break;
    }

    return QVariant();
}

ItemParser::QcdmDataItem::QcdmDataItem(const QJsonObject &obj)
    : ItemParser::DataItem(obj),
      mID(0),
      mDataType(ValueTypeNone),
      mData(0)
{
    if(obj.contains("id")) {
        mID = obj["id"].toInt();
    }
    if(obj.contains("datatype")) {
        mDataType = getValueType(obj["datatype"].toString());
    }
    if(obj.contains("data")) {
        mData = getValue(obj["data"].toString(), mDataType);
    }
}

Serial::SerialCommand* ItemParser::QcdmDataItem::getCommand(Serial::SerialDevice* device) {
    Serial::SerialCommand* cmd = new Serial::QCDM::Commands::QcdmCommand(device, static_cast<Serial::QCDM::DiagCommands>(id()), data().toByteArray());
    if(cmd != nullptr) {
        cmd->setOffset(offset());
    }
    return cmd;
}

QString ItemParser::QcdmDataItem::hashID() const {
    QString ret = QString::number(mID);
    switch(mDataType) {
    case ValueType8Bit:
    case ValueType16Bit:
    case ValueType32Bit:
        ret += QString::number(mData.toUInt());
    case ValueType64Bit:
        ret += QString::number(mData.toULongLong());
        break;
    case ValueTypeString:
        ret += mData.toString();
        break;
    case ValueTypeRaw:
        ret += QString(mData.toString().toLatin1().toHex());
    default:
        break;
    }
    return ret;
}

ItemParser::NvDataItem::NvDataItem(const QJsonObject &obj)
    : ItemParser::QcdmDataItem(obj)
{

}

Serial::SerialCommand* ItemParser::NvDataItem::getCommand(SerialDevice *device) {
    Serial::SerialCommand* cmd = nullptr;
    Serial::QCDM::NvItem id = static_cast<Serial::QCDM::NvItem>(QcdmDataItem::id());
    switch(valueType()) {
        case ValueType8Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand8Bit(device, true, id, data().toByteArray());
            break;
        }
        case ValueType16Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand16Bit(device, true, id, data().toByteArray());
            break;
        }
        case ValueType32Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand32Bit(device, true, id, data().toByteArray());
            break;
        }
        case ValueType64Bit: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand64Bit(device, true, id, data().toByteArray());
            break;
        }
        case ValueTypeString: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommandString(device, true, id, data().toByteArray());
            break;
        }
        case ValueTypeRaw: {
            cmd = new Serial::QCDM::Commands::Nv::NvCommand(device, true, id, data().toByteArray());
            break;
        }
        default: {
            break;
        }
    }

    if(cmd != nullptr) {
        cmd->setOffset(QcdmDataItem::offset());
    }
    return cmd;
}

SerialItemParser::SerialItemParser(SerialDevice* device)
    : mDevice(device)
{

}

bool SerialItemParser::checkItem(const QJsonObject &obj) {
    ItemParser::DataItem* dataItem = getDataItem(obj);
    if(dataItem == nullptr) {
        return false;
    }

    QString hashID = dataItem->hashID();
    if(mCachedData.contains(hashID)) {
        bool r = dataItem->equals(mCachedData[hashID]);
        delete dataItem;
        return r;
    }

    Serial::SerialCommand *cmd = dataItem->getCommand(mDevice);
    if(cmd == nullptr) {
        delete dataItem;
        return false;
    }

    if(obj.contains("offset")) {
        cmd->setOffset(obj["offset"].toInt());
    }

    cmd->execute();
    if(!cmd->resultSuccess()) {
        delete cmd;
        delete dataItem;
        return false;
    }

    QVariant outData = cmd->result()->data();
    mCachedData.insert(hashID, outData);

    delete cmd;

    return dataItem->equals(outData);
}

ItemParser::DataItem *SerialItemParser::getDataItem(const QJsonObject &obj) {
    if(!obj.contains("type")) {
        return nullptr;
    }

    ItemParser::ItemType itemType = ItemParser::DataItem::getItemType(obj["type"].toString());
    if(itemType == ItemParser::ItemTypeNone) {
        return nullptr;
    }

    switch(itemType) {
    case ItemParser::ItemTypeQCDM:
        return new ItemParser::QcdmDataItem(obj);
    case ItemParser::ItemTypeNV:
        return new ItemParser::NvDataItem(obj);
    default:
        break;
    }

    return nullptr;
}
