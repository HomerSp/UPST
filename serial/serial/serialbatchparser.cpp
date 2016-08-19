#include <QDebug>
#include <QStringList>
#include <QTextStream>

#include "serialdevice.h"
#include "serialbatchparser.h"

using namespace Serial;

SerialBatchParser::SerialBatchParser(QString data) {
    uint8_t meidColumn, imeiColumn, esnColumn, minColumn, mdnColumn, spcColumn;
    meidColumn = imeiColumn = esnColumn = minColumn = mdnColumn = spcColumn = 0xFF;

    QTextStream stream(&data, QIODevice::ReadOnly);

    QString header = stream.readLine();
    QStringList headerList = header.split(',');
    for(int i = 0; i < headerList.size(); i++) {
        if(headerList.at(i).startsWith("MEID", Qt::CaseInsensitive)) {
            meidColumn = i;
        } else if(headerList.at(i).startsWith("IMEI", Qt::CaseInsensitive)) {
            imeiColumn = i;
        } else if(headerList.at(i).startsWith("ESN", Qt::CaseInsensitive)) {
            esnColumn = i;
        } else if(headerList.at(i).startsWith("MIN", Qt::CaseInsensitive)) {
            minColumn = i;
        } else if(headerList.at(i).startsWith("MDN", Qt::CaseInsensitive)) {
            mdnColumn = i;
        } else if(headerList.at(i).startsWith("SPC", Qt::CaseInsensitive)) {
            spcColumn = i;
        }
    }

    // No MEID, IMEI or ESN column found.
    if(meidColumn == 0xFF && imeiColumn == 0xFF && esnColumn == 0xFF) {
        return;
    }

    // No MIN or MDN column found.
    if(minColumn == 0xFF && mdnColumn == 0xFF && spcColumn == 0xFF) {
        return;
    }

    // MIN = MDN
    if(minColumn == 0xFF) {
        minColumn = mdnColumn;
    }

    while(!stream.atEnd()) {
        QString line = stream.readLine();

        SerialBatchItem *item = new SerialBatchItem;
        item->ID = 0;
        item->Type = SerialBatchItemTypeNone;
        item->MIN = 0;
        item->MDN = "";
        item->SPC = "";

        QStringList lineList = line.split(',');

        bool ok;
        if(meidColumn != 0xFF && meidColumn < lineList.size()) {
            QString s = lineList.at(meidColumn);
            if(s.length() == 14) {
                item->ID = s.toULongLong(&ok, 16);
            } else {
                uint64_t first = s.mid(0, 10).toULongLong(&ok, 10);
                uint64_t last = s.mid(10, 8).toULongLong(&ok, 10);
                item->ID = QString(QString(QByteArray::number(first, 16)).leftJustified(8, '0') + QString(QByteArray::number(last, 16)).leftJustified(6, '0')).toULongLong(&ok, 16);
            }
            if(ok) {
                item->Type = SerialBatchItemTypeMEID;
            }
        }
        if(item->Type == SerialBatchItemTypeNone && imeiColumn != 0xFF && imeiColumn < lineList.size()) {
            item->ID = lineList.at(imeiColumn).toULongLong(&ok, 16);
            if(ok) {
                item->Type = SerialBatchItemTypeIMEI;
            }
        }
        if(item->Type == SerialBatchItemTypeNone && esnColumn != 0xFF && esnColumn < lineList.size()) {
            item->ID = lineList.at(esnColumn).toULongLong(&ok, 16);
            if(ok) {
                item->Type = SerialBatchItemTypeESN;
            }
        }

        // No point in continuing if we don't have a valid device ID.
        if(item->Type == SerialBatchItemTypeNone) {
            delete item;
            continue;
        }

        if(minColumn < lineList.size()) {
            item->MIN = lineList.at(minColumn).toULongLong();
        }
        if(mdnColumn < lineList.size()) {
            item->MDN = lineList.at(mdnColumn);
        }
        if(spcColumn != 0xFF && spcColumn < lineList.size()) {
            item->SPC = lineList.at(spcColumn);
        }

        // Just in case.
        if(item->MIN == 0 && item->MDN.size() > 0) {
            item->MIN = item->MDN.toULongLong();
        }

        mItems.append(item);
    }
}

SerialBatchParser::~SerialBatchParser() {
    foreach(SerialBatchItem* item, mItems) {
        delete item;
    }
}

bool SerialBatchParser::updateDevice(SerialDevice* device) {
    foreach(SerialBatchItem* item, mItems) {
        switch(item->Type) {
        case SerialBatchItemTypeMEID: {
            if(device->meid() != item->ID) {
                continue;
            }

            break;
        }
        case SerialBatchItemTypeIMEI: {
            if(device->imei() != item->ID) {
                continue;
            }

            break;
        }
        case SerialBatchItemTypeESN: {
            if(device->esn() != item->ID) {
                continue;
            }

            break;
        }
        default: {
            continue;
        }
        }

        device->setProvisionData(item->MDN, item->MIN, item->SPC);
        return true;
    }

    return false;
}
