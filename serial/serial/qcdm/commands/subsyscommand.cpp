#include "subsyscommand.h"

using namespace Serial::QCDM::Commands;

SubSysCommand::SubSysCommand(Serial::SerialDevice* device, Serial::QCDM::DiagSubSysID id, uint16_t code, QByteArray data)
    : QcdmCommand(device),
      mID(id),
      mCode(code)
{
    QByteArray writeData;
    writeData.append(static_cast<uint8_t>(id));
    writeData.append(static_cast<uint8_t>((code) & 0xFF));
    writeData.append(static_cast<uint8_t>((code >> 8) & 0xFF));
    writeData.append(data);

    addItem(new QcdmCommandItem(QCDM::DIAG_SUBSYS_CMD_F, writeData));
}

void SubSysCommand::execute() {
    QcdmCommand::execute(false);

    SerialCommandResult* result = SerialCommand::results().first();
    if(!result->success()) {
        return;
    }

    QByteArray resultData = result->data().toByteArray();
    if(resultData.size() < 1) {
        result->setSuccess(false);
        return;
    }

    uint8_t id = static_cast<uint8_t>(resultData.at(0));
    if(id != mID) {
        result->setSuccess(false);
        return;
    }

    uint16_t code = ((uint16_t)resultData.at(2) << 8) | (uint8_t)resultData.at(1);
    if(code != mCode) {
        result->setSuccess(false);
        return;
    }

    resultData.remove(0, 3);
    if(offset() > 0) {
        resultData.remove(0, offset());
    }

    result->setData(resultData);
}
