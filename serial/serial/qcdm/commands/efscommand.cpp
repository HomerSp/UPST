#include "efscommand.h"

using namespace Serial::QCDM::Commands;

EFSCommand::EFSCommand(Serial::SerialDevice* device, Serial::QCDM::DiagEFSCommandCode code, QByteArray data)
    : SubSysCommand(device, DIAG_SUBSYS_FS, code, data)
{

}
