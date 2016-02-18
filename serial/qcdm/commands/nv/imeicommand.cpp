#include <QString>

#include "imeicommand.h"

using namespace Serial::QCDM::Commands::Nv;

IMEICommand::IMEICommand(SerialDevice* device)
    : NvCommand(device, true, NvItem::NV_UE_IMEI_I)
{

}

bool IMEICommand::execute(uint64_t& result, uint16_t* errorCode) {
    QByteArray ret;
    if(!NvCommand::execute(ret, errorCode)) {
        return false;
    }

    if(ret.length() < 9 || ret.at(0) != 0x08) {
        return false;
    }

    result = 0;
    for(int i = 0; i < 8; i++) {
        uint64_t c = ret.at(8 - i);
        if(i < 1) {
            result |= (c & 0x0F) << (i * 8);
        }

        if(i > 0) {
            result |= ((c >> 4) & 0x0F) << ((i * 8) - 4);
        }
    }

    return true;
}

