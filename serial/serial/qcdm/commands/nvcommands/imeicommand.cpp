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

            if(i < 7) {
                uint64_t d = ((c >> 4) & 0x0F) | ((c & 0x0F) << 4);
                res |= (d & 0xFF) << (i * 8);
            } else {
                res |= (((c >> 4) & 0x0F) << ((i * 8) + 4)) >> 4;
            }
        }

        result->setData((quint64)res);
    }
}

