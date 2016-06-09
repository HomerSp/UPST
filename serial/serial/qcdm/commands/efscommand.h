#ifndef EFSCOMMAND_H
#define EFSCOMMAND_H

#include <QByteArray>
#include "../../qcdm/efs.h"
#include "../commands/subsyscommand.h"

namespace Serial {
    class SerialDevice;

    namespace QCDM {
        namespace Commands {
            class EFSCommand : public SubSysCommand
            {
            public:
                EFSCommand(SerialDevice* device, Serial::QCDM::DiagEFSCommandCode code, QByteArray data = 0);
            };
        }
    }
}

#endif // EFSCOMMAND_H
