#include "aoccommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

AOCCommand::AOCCommand(SerialDevice* device, bool read)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, QCDM::NV_ACCOLC_I));
    } else {
        char m = device->minStr().mid(device->minStr().size() - 1, 1).at(0).toLatin1();

        QByteArray data;
        data.append((char)0x0);
        data.append(m);
        data.append(m);
        addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_ACCOLC_I, data));
    }
}

