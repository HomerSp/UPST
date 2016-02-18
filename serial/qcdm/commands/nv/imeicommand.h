#ifndef IMEICOMMAND
#define IMEICOMMAND

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class IMEICommand : public NvCommand
                {
                public:
                    IMEICommand(SerialDevice* device);

                    bool execute(uint64_t& result, uint16_t* errorCode = nullptr);
                };
            }
        }
    }
}

#endif // IMEICOMMAND

