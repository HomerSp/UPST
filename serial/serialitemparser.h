#ifndef SERIALITEMPARSER_H
#define SERIALITEMPARSER_H

#include "serialdevice.h"

namespace Serial {
    class SerialItemParser
    {
    public:
        SerialItemParser(SerialDevice* device);

        bool checkItem(const QJsonObject& obj);
        Serial::SerialCommand* getItem(const QJsonObject& obj);

    private:
        enum ItemType {
            ItemTypeNone,
            ItemTypeAT,
            ItemTypeQCDM,
            ItemTypeNV,
        };

        enum DataType {
            DataTypeNone,
            DataType8Bit,
            DataType16Bit,
            DataType32Bit,
            DataType64Bit,
            DataTypeString,
            DataTypeRaw,
        };

        bool checkItem(SerialDevice* device, ItemType itemType, DataType type, const QVariant& checkData, QVariant& outData, uint16_t id = 0);
        Serial::SerialCommand* getItem(const QJsonObject& obj, bool isCheck, QVariant& checkData, uint16_t& id);

        bool nvDataEquals(const QVariant& checkData, const QVariant& nvData);

        ItemType getItemType(const QString& itemType);
        Serial::SerialCommand* getItem(ItemType itemType, DataType dataType, QVariant checkData = 0, uint16_t id = 0);
        QVariant getValue(const QString& type, const QString& checkData, DataType& outType);

        SerialDevice* mDevice;
        QHash<int, QVariant> mCachedData;
    };
}

#endif // SERIALITEMPARSER_H
