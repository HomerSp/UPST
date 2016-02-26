#include <QDebug>
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
        SerialCommand::result()->setData(result);
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

    uint min1 = min1Data.toHex().toULong(0, 16);
    uint min2 = min2Data.toHex().toULong(0, 16);
    min2 = (min2 + 1) % 10 + (((((min2 % 100) / 10) + 1) % 10) * 10) + ((((min2 / 100) + 1) % 10) * 100);

    uint min1a = (uint) (min1 & 0xffc000) >> 14;
    min1a = (min1a + 1) % 10 + (((((min1a % 100) / 10) + 1) % 10) * 10) + ((((min1a / 100) + 1) % 10) * 100);

    uint min1b = (uint) ((min1 & 0x3c00) >> 10) % 10;

    uint min1c = (uint) (min1 & 0x3ff);
    min1c = ((min1c + 1) % 10) + (((((min1c % 100) / 10) + 1) % 10) * 10) + ((((min1c / 100) + 1) % 10) * 100);

    QString min = QString("%1%2%3%4").arg(min2, 3, 10, QChar('0')).arg(min1a, 3, 10, QChar('0')).arg(min1b, 1, 10, QChar('0')).arg(min1c, 3, 10, QChar('0'));

    bool ok = false;
    result = min.toULongLong(&ok);

    return ok;
}
