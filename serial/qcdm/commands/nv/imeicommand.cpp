#include <QString>

#include "imeicommand.h"

using namespace Serial::QCDM::Commands::Nv;

IMEICommand::IMEICommand(SerialDevice* device)
    : NvCommand(device, true, NvItem::NV_UE_IMEI_I)
{

}

void IMEICommand::execute() {
    NvCommand::execute();

    const QList<SerialCommandResult*> results = SerialCommand::results();
    foreach(SerialCommandResult* result, results) {
        if(!result->success()) {
            continue;
        }

        QByteArray ret = result->data().toByteArray();
        if(ret.length() < 9 || ret.at(0) != 0x08) {
            result->setSuccess(false);
            continue;
        }

        uint64_t res = 0;
        for(int i = 0; i < 8; i++) {
            uint64_t c = ret.at(8 - i);
            if(i < 1) {
                res |= (c & 0x0F) << (i * 8);
            }

            if(i > 0) {
                res |= ((c >> 4) & 0x0F) << ((i * 8) - 4);
            }
        }

        result->setData(res);
    }
}

