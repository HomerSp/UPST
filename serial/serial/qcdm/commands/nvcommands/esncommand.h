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
                    ESNCommand(SerialDevice* device, bool read = true, uint32_t data = 0)
                        : NvCommand32Bit(device, read, NvItem::NV_ESN_I, data) {

                    }

                };
            }
        }
    }
}

#endif // ESNCOMMAND_H
