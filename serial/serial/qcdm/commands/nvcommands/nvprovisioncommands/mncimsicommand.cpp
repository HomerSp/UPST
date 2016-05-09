#include "../../../../serialprovisiondata.h"
#include "mncimsicommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

MncImsiCommand::MncImsiCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue)
    : NvCommand(device)
{
    if(!read) {
        if(jsonValue != nullptr) {
            uint8_t val = NvCommand::getEncodedValue(jsonValue->toString().toUInt(), 2);

            QByteArray data;
            data.append((char)0x0);
            data.append(static_cast<char>((val) & 0xFF));

            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_IMSI_11_12_I, data));
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_IMSI_T_11_12_I, data));
        }
    }
}
