#include "nvcommand.h"

#include <QDebug>

using namespace Serial::QCDM::Commands::Nv;

NvCommand::NvCommand(SerialDevice* device, bool read, QCDM::NvItem item, QByteArray data)
    : QcdmCommand(device, new NvCommandItem((read)?QCDM::DIAG_NV_READ_F:QCDM::DIAG_NV_WRITE_F, item, data))
{

}

NvCommand::NvCommand(SerialDevice* device, const QList<NvCommandItem*>& cmds)
    : QcdmCommand(device)
{
    QList<QcdmCommandItem*> items;
    foreach(NvCommandItem* item, cmds) {
        addItem(item);
    }
}

void NvCommand::execute() {
    QcdmCommand::execute();

    const QList<SerialCommandResult*> &results = SerialCommand::results();
    for(int i = 0; i < results.size(); i++) {
        SerialCommandResult* result = results.at(i);
        if(!result->success()) {
            continue;
        }

        QByteArray resultData = result->data().toByteArray();
        if(resultData.size() < 2) {
            result->setSuccess(false);
            continue;
        }

        uint16_t item = ((uint16_t)resultData.at(1) << 8) | (uint8_t)resultData.at(0);
        QCDM::NvItem nvItem = this->nvItem(i);
        if(item != nvItem) {
            qDebug()<<"NvCommand item"<<item<<"!="<<nvItem;
            result->setSuccess(false);
            continue;
        }

        resultData.remove(0, 2);

        if(resultData.size() < 2) {
            result->setSuccess(false);
            continue;
        }

        uint16_t error = ((uint16_t)resultData.at(resultData.size() - 3) << 8) | (uint8_t)resultData.at(resultData.size() - 2);
        if(error != 0) {
            result->setErrorCode(error);
            result->setSuccess(false);
            continue;
        }

        result->setData(resultData);
    }
}

bool NvCommand::getRequest(QList<QByteArray> &request) {
    for(int i = 0; i < count(); i++) {
        QByteArray reqData;
        uint16_t nvValue = (uint16_t)this->nvItem(i);
        uint8_t nvByte1 = (uint8_t)((nvValue) & 0xFF);
        uint8_t nvByte2 = (uint8_t)((nvValue >> 8) & 0xFF);

        reqData.append(command(i));
        reqData.append(nvByte1);
        reqData.append(nvByte2);
        reqData.append(data(i));

        while(reqData.size() < 133) {
            reqData.append(static_cast<char>(0x00));
        }

        Serial::CRCUtils::addCRC(reqData);

        request.append(reqData);
    }

    return true;
}

uint64_t NvCommand::getEncodedValue(uint64_t val, uint8_t width) {
    uint64_t valModified = 0;
    for(int i = 0; i < width; i++) {
        uint8_t c = ((static_cast<uint16_t>(val / pow(10.0f, i)) % 10) + 9) % 10;
        valModified += c * pow(10.0f, i);
    }

    return valModified;
}

quint64 byteArrayToInt64(const QByteArray &arr, unsigned int length) {
    quint64 ret = 0;

    for(unsigned int i = 0; i < length; i++) {
        quint64 a = arr.at(i);
        ret |= ((a & 0xFF) << (i * 8));
    }

    return ret;
}

NvCommand8Bit::NvCommand8Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint8_t data)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, item));
    } else {
        QByteArray dataArr;
        dataArr.append(uint8_t(data & 0xFF));

        addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, item, dataArr));
    }
}

void NvCommand8Bit::execute() {
    NvCommand::execute();

    const QList<SerialCommandResult*> &results = SerialCommand::results();
    for(int i = 0; i < results.size(); i++) {
        SerialCommandResult* result = results.at(i);
        if(!result->success()) {
            continue;
        }

        QByteArray resultData = result->data().toByteArray();
        if(resultData.size() < 1) {
            result->setSuccess(false);
            continue;
        }

        uint8_t data = (uint8_t)byteArrayToInt64(resultData, 1);
        result->setData(data);
        result->setSuccess(true);
    }
}

NvCommand16Bit::NvCommand16Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint16_t data)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, item));
    } else {
        QByteArray dataArr;
        dataArr.append(uint8_t(data & 0xFF));
        dataArr.append(uint8_t((data >> 8) & 0xFF));

        addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, item, dataArr));
    }
}

void NvCommand16Bit::execute() {
    NvCommand::execute();

    const QList<SerialCommandResult*> &results = SerialCommand::results();
    for(int i = 0; i < results.size(); i++) {
        SerialCommandResult* result = results.at(i);
        if(!result->success()) {
            continue;
        }

        QByteArray resultData = result->data().toByteArray();
        if(resultData.size() < 2) {
            result->setSuccess(false);
            continue;
        }

        uint16_t data = (uint16_t)byteArrayToInt64(resultData, 2);
        result->setData(data);
        result->setSuccess(true);
    }
}

NvCommand32Bit::NvCommand32Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint32_t data)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, item));
    } else {
        QByteArray dataArr;
        dataArr.append(uint8_t(data & 0xFF));
        dataArr.append(uint8_t((data >> 8) & 0xFF));
        dataArr.append(uint8_t((data >> 16) & 0xFF));
        dataArr.append(uint8_t((data >> 24) & 0xFF));

        addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, item, dataArr));
    }
}

void NvCommand32Bit::execute() {
    NvCommand::execute();

    const QList<SerialCommandResult*> &results = SerialCommand::results();
    for(int i = 0; i < results.size(); i++) {
        SerialCommandResult* result = results.at(i);
        if(!result->success()) {
            continue;
        }

        QByteArray resultData = result->data().toByteArray();
        if(resultData.size() < 4) {
            result->setSuccess(false);
            continue;
        }

        uint32_t data = (uint32_t)byteArrayToInt64(resultData, 4);
        result->setData((quint64)data);
        result->setSuccess(true);
    }
}

NvCommand64Bit::NvCommand64Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint64_t data)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, item));
    } else {
        QByteArray dataArr;
        dataArr.append(uint8_t(data & 0xFF));
        dataArr.append(uint8_t((data >> 8) & 0xFF));
        dataArr.append(uint8_t((data >> 16) & 0xFF));
        dataArr.append(uint8_t((data >> 24) & 0xFF));
        dataArr.append(uint8_t((data >> 32) & 0xFF));
        dataArr.append(uint8_t((data >> 40) & 0xFF));
        dataArr.append(uint8_t((data >> 48) & 0xFF));
        dataArr.append(uint8_t((data >> 56) & 0xFF));

        addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, item, dataArr));
    }
}

void NvCommand64Bit::execute() {
    NvCommand::execute();

    const QList<SerialCommandResult*> &results = SerialCommand::results();
    for(int i = 0; i < results.size(); i++) {
        SerialCommandResult* result = results.at(i);
        if(!result->success()) {
            continue;
        }

        QByteArray resultData = result->data().toByteArray();
        if(resultData.size() < 8) {
            result->setSuccess(false);
            continue;
        }

        quint64 data = (quint64)byteArrayToInt64(resultData, 8);
        result->setData(data);
        result->setSuccess(true);
    }
}


void NvCommandString::execute() {
    NvCommand::execute();

    const QList<SerialCommandResult*> &results = SerialCommand::results();
    for(int i = 0; i < results.size(); i++) {
        SerialCommandResult* result = results.at(i);
        if(!result->success()) {
            continue;
        }

        QByteArray resultData = result->data().toByteArray();
        QString data = "";
        for(int x = 1; x < resultData.size(); x++) {
            if(resultData[x] == '\0') {
                break;
            }

            data += resultData[x];
        }

        result->setData(data);
        result->setSuccess(true);
    }
}
