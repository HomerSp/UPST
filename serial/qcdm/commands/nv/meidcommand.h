#ifndef MEIDCOMMAND_H
#define MEIDCOMMAND_H

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class MEIDCommand : public NvCommand64Bit
                {
                public:
                    MEIDCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QByteArray data = 0)
                        : NvCommand64Bit(communicator, cmd, NvItem::NV_MEID_I, data) {

                    }
                };
            }
        }
    }
}

#endif // MEIDCOMMAND_H
