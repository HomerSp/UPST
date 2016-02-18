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

bool NvCommand::execute(QList<QByteArray>& result, uint16_t* errorCode) {
    if(!QcdmCommand::execute(result, errorCode)) {
        return false;
    }

    for(int i = 0; i < result.size(); i++) {
        QByteArray &resultData = result[i];
        if(resultData.size() < 2) {
            return false;
        }

        uint16_t item = ((uint16_t)resultData.at(1) << 8) | (uint8_t)resultData.at(0);
        QCDM::NvItem nvItem = this->nvItem(i);
        if(item != nvItem) {
            qDebug()<<"NvCommand item"<<item<<"!="<<nvItem;
            return false;
        }

        resultData.remove(0, 2);

        if(resultData.size() < 2) {
            return false;
        }

        uint16_t error = ((uint16_t)resultData.at(resultData.size() - 3) << 8) | (uint8_t)resultData.at(resultData.size() - 2);
        if(errorCode != nullptr) {
            *errorCode = error;
        }

        if(error != 0) {
            return false;
        }
    }

    return true;
}

bool NvCommand::execute(QByteArray& result, uint16_t* errorCode) {
    QList<QByteArray> results;
    if(!execute(results, errorCode)) {
        return false;
    }

    result = results[0];

    return true;
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

uint64_t byteArrayToInt64(const QByteArray &arr, unsigned int length) {
    uint64_t ret = 0;

    for(unsigned int i = 0; i < length; i++) {
        uint64_t a = arr.at(i);
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

bool NvCommand8Bit::execute(uint8_t& result, uint16_t* errorCode) {
    QByteArray ret;
    if(!NvCommand::execute(ret, errorCode)) {
        return false;
    }

    if(ret.size() < 1) {
        return false;
    }

    result = (uint8_t)byteArrayToInt64(ret, 1);
    return true;
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

bool NvCommand16Bit::execute(uint16_t& result, uint16_t* errorCode) {
    QByteArray ret;
    if(!NvCommand::execute(ret, errorCode)) {
        return false;
    }

    if(ret.size() < 2) {
        return false;
    }

    result = (uint16_t)byteArrayToInt64(ret, 2);
    return true;
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

bool NvCommand32Bit::execute(uint32_t& result, uint16_t* errorCode) {
    QByteArray ret;
    if(!NvCommand::execute(ret, errorCode)) {
        return false;
    }

    if(ret.size() < 4) {
        return false;
    }

    result = (uint32_t)byteArrayToInt64(ret, 4);
    return true;
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

bool NvCommand64Bit::execute(uint64_t& result, uint16_t* errorCode) {
    QByteArray ret;
    if(!NvCommand::execute(ret, errorCode)) {
        return false;
    }

    if(ret.size() < 8) {
        return false;
    }

    result = (uint64_t)byteArrayToInt64(ret, 8);

    return true;
}


bool NvCommandString::execute(QString& result, uint16_t* errorCode) {
    QByteArray ret;
    if(!NvCommand::execute(ret, errorCode)) {
        return false;
    }

    result = "";
    for(int i = 1; i < ret.size(); i++) {
        if(ret[i] == '\0') {
            break;
        }

        result += ret[i];
    }

    return true;
}
