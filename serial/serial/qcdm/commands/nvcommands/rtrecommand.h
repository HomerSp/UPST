#ifndef RTRECOMMAND_H
#define RTRECOMMAND_H

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class RTRECommand : public NvCommand8Bit
                {
                public:
                    RTRECommand(SerialDevice* device, bool read = true, uint8_t mode = 0)
                        : NvCommand8Bit(device, read, NV_RTRE_CONFIG_I, mode)  {

                    }

                    RTREMode mode() {
                        return static_cast<RTREMode>(resultData());
                    }
                };
            }
        }
    }
}

#endif // RTRECOMMAND_H
