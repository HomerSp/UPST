#ifndef SERIALITEMPARSER_H
#define SERIALITEMPARSER_H

#include "serialdevice.h"

namespace Serial {
    namespace ItemParser {
        enum ItemType {
            ItemTypeNone,
            ItemTypeAT,
            ItemTypeQCDM,
            ItemTypeNV,
        };

        enum ValueType {
            ValueTypeNone,
            ValueType8Bit,
            ValueType16Bit,
            ValueType32Bit,
            ValueType64Bit,
            ValueTypeString,
            ValueTypeRaw,
        };

        class DataItem {
        public:
            DataItem(const QJsonObject& obj);
            virtual ~DataItem() {

            }

            bool equals(const QVariant& value);

            virtual Serial::SerialCommand* getCommand(Serial::SerialDevice* device) = 0;

            virtual QString hashID() const = 0;

            static ItemType getItemType(const QString& type);

        protected:
            ItemType itemType() const {
                return mItemType;
            }
            int offset() const {
                return mOffset;
            }
            ValueType valueType() const {
                return mValueType;
            }
            QVariant value() const {
                return mValue;
            }

            ValueType getValueType(const QString& type) const;
            QVariant getValue(const QString& value, ValueType type) const;

        private:
            ItemType mItemType;

            int mOffset;
            ValueType mValueType;
            QVariant mValue;
        };

        class QcdmDataItem : public DataItem {
        public:
            QcdmDataItem(const QJsonObject& obj);
            virtual ~QcdmDataItem() {

            }

            virtual Serial::SerialCommand* getCommand(Serial::SerialDevice* device);

            virtual QString hashID() const;

        protected:
            int id() const {
                return mID;
            }
            QByteArray data() const {
                return mData;
            }

        private:
            int mID;

            QByteArray mData;
        };

        class NvDataItem : public QcdmDataItem {
        public:
            NvDataItem(const QJsonObject& obj);
            virtual ~NvDataItem() {

            }

            virtual Serial::SerialCommand* getCommand(SerialDevice *device);
        };
    }

    class SerialItemParser
    {
    public:
        SerialItemParser(SerialDevice* device);

        bool checkItem(const QJsonObject& obj);

    private:
        ItemParser::DataItem* getDataItem(const QJsonObject& obj);

        SerialDevice* mDevice;
        QHash<QString, QVariant> mCachedData;
    };
}

#endif // SERIALITEMPARSER_H
