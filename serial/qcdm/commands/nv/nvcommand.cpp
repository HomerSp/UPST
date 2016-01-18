#include "nvcommand.h"

#include <QDebug>

using namespace Serial::QCDM::Commands::Nv;

NvCommand::NvCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QCDM::NvItem item, QByteArray data)
    : QcdmCommand(communicator, cmd, data),
      mItem(item)
{

}

bool NvCommand::execute(QByteArray& result, uint16_t* errorCode) {
    if(!QcdmCommand::execute(result, errorCode)) {
        return false;
    }

    if(result.size() < 2) {
        return false;
    }

    uint16_t item = ((uint16_t)result.at(1) << 8) | (uint8_t)result.at(0);
    if(item != mItem) {
        qDebug()<<"NvCommand item"<<item<<"!="<<mItem;
        return false;
    }

    result.remove(0, 2);

    if(result.size() < 2) {
        return true;
    }

    uint16_t error = ((uint16_t)result.at(result.size() - 3) << 8) | (uint8_t)result.at(result.size() - 2);
    if(errorCode != nullptr) {
        *errorCode = error;
    }

    return error == 0;
}

bool NvCommand::getRequest(QByteArray &request) {
    uint16_t nvValue = (uint16_t)mItem;
    uint8_t nvByte1 = (uint8_t)((nvValue) & 0xFF);
    uint8_t nvByte2 = (uint8_t)((nvValue >> 8) & 0xFF);

    request.append(command());
    request.append(nvByte1);
    request.append(nvByte2);
    request.append(data());

    while(request.size() < 133) {
        request.append(static_cast<char>(0x00));
    }

    Serial::CRCUtils::addCRC(request);

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
