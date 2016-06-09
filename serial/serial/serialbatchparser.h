#ifndef SERIAL_SERIALBATCHPARSER_H
#define SERIAL_SERIALBATCHPARSER_H

#include <QString>

namespace Serial {
    class SerialDevice;

    class SerialBatchParser
    {
    public:
        SerialBatchParser(QString data);
        ~SerialBatchParser();

        bool updateDevice(SerialDevice* device);

    private:
        enum SerialBatchItemType {
            SerialBatchItemTypeNone = 0,
            SerialBatchItemTypeMEID,
            SerialBatchItemTypeIMEI,
            SerialBatchItemTypeESN,
        };

        struct SerialBatchItem {
            SerialBatchItemType Type;
            uint64_t ID;

            uint64_t MIN;
            QString MDN;
            QString SPC;
        };

        QList<SerialBatchItem*> mItems;
    };
}

#endif // SERIAL_SERIALBATCHPARSER_H
