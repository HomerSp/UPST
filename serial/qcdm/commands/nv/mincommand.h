#ifndef MINCOMMAND_H
#define MINCOMMAND_H

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class MINCommand : public NvCommand
                {
                public:
                    MINCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QString data = 0);

                    virtual bool execute(uint64_t &result, uint16_t* errorCode = nullptr);
                };
            }
        }
    }
}

#endif // MINCOMMAND_H
