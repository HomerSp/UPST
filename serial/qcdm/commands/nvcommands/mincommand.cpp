#include <QDebug>
#include <QDataStream>
#include <QString>

#include "mincommand.h"

using namespace Serial::QCDM::Commands::Nv;

MINCommand::MINCommand(SerialDevice* device, bool read, uint64_t data)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, QCDM::NvItem::NV_MIN1_I));
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, QCDM::NvItem::NV_MIN2_I));
    } else {
        QList<QByteArray> dataArr;
        if(toNv(data, dataArr)) {
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NvItem::NV_MIN1_I, dataArr[0]));
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NvItem::NV_MIN2_I, dataArr[1]));
        }
    }
}

void MINCommand::execute() {
    NvCommand::execute();

    QList<QByteArray> data;

    const QList<SerialCommandResult*> &results = SerialCommand::results();
    foreach(SerialCommandResult* result, results) {
        if(!result->success()) {
            continue;
        }

        data.append(result->data().toByteArray());
    }

    if(data.size() != 2) {
        return;
    }

    uint64_t result = 0;
    if(fromNv(data, result)) {
        SerialCommand::result()->setData((quint64)result);
    }
}

bool MINCommand::fromNv(const QList<QByteArray>& data, uint64_t& result) {
    QByteArray min1Data;
    min1Data.append(data[0][8]);
    min1Data.append(data[0][7]);
    min1Data.append(data[0][6]);
    min1Data.append(data[0][5]);

    QByteArray min2Data;
    min2Data.append(data[1][4]);
    min2Data.append(data[1][3]);

    uint64_t min1 = min1Data.toHex().toULong(0, 16);
    uint64_t min2 = getDecodedValue(min2Data.toHex().toULong(0, 16), 3);

    uint min1a = getDecodedValue((min1 & 0xffc000) >> 14, 3);
    uint min1b = ((min1 & 0x3c00) >> 10) % 10;
    uint min1c = getDecodedValue(min1 & 0x3ff, 3);

    qDebug()<<"min1a"<<min1a<<"min1b"<<min1b<<"min1c"<<min1c<<"min2"<<min2;

    result = min1c + (min1b * 1000ULL) + (min1a * 10000ULL) + (min2 * 10000000ULL);

    return true;
}

bool MINCommand::toNv(uint64_t data, QList<QByteArray>& result) {
    uint64_t min2 = NvCommand::getEncodedValue((data / 10000000ULL) % 1000, 3);
    uint64_t min1a = NvCommand::getEncodedValue((data / 10000ULL) % 1000, 3);
    uint64_t min1b = (data / 1000ULL) % 10;
    uint64_t min1c = NvCommand::getEncodedValue((data) % 1000, 3);

    if(min1b == 0) {
        min1b = 10;
    }

    uint min1 = min1c + (min1b << 10) + (min1a << 14);

    {
        QByteArray minArr;
        minArr.fill('\0', 8);
        minArr[8] = (min1 >> 24) & 0xFF;
        minArr[7] = (min1 >> 16) & 0xFF;
        minArr[6] = (min1 >> 8) & 0xFF;
        minArr[5] = (min1) & 0xFF;

        result.append(minArr);
    }
    {
        QByteArray minArr;
        minArr.fill('\0', 4);
        minArr[4] = (min2 >> 8) & 0xFF;
        minArr[3] = (min2) & 0xFF;
        result.append(minArr);
    }

    qDebug()<<"toNv min1Data"<<QString(result.at(0).toHex())<<"min2Data"<<QString(result.at(1).toHex());

    return true;
}
