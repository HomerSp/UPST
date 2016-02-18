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
                    MEIDCommand(SerialDevice* device, bool read = true, uint64_t data = 0)
                        : NvCommand64Bit(device, read, NvItem::NV_MEID_I, data) {

                    }
                };
            }
        }
    }
}

#endif // MEIDCOMMAND_H
