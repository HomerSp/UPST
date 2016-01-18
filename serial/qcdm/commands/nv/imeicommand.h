#ifndef IMEICOMMAND
#define IMEICOMMAND

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class IMEICommand : public NvCommand64Bit
                {
                public:
                    IMEICommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QByteArray data = 0);

                    bool execute(uint64_t& result, uint16_t* errorCode = nullptr);
                };
            }
        }
    }
}

#endif // IMEICOMMAND

