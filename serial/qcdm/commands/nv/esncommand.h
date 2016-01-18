#ifndef ESNCOMMAND_H
#define ESNCOMMAND_H



#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class ESNCommand : public NvCommand32Bit
                {
                public:
                    ESNCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QByteArray data = 0)
                        : NvCommand32Bit(communicator, cmd, NvItem::NV_ESN_I, data) {

                    }

                };
            }
        }
    }
}

#endif // ESNCOMMAND_H
