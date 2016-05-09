#include "../../../../serialprovisiondata.h"
#include "mccimsicommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

MccImsiCommand::MccImsiCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue)
    : NvCommand(device)
{
    if(!read) {
        if(jsonValue != nullptr) {
            uint16_t val = NvCommand::getEncodedValue(jsonValue->toString().toUInt(), 3);

            QByteArray data;
            data.append((char)0x0);
            data.append(static_cast<char>((val) & 0xFF));
            data.append(static_cast<char>((val >> 8) & 0xFF));

            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_IMSI_MCC_I, data));
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_IMSI_T_MCC_I, data));
        }
    }
}
