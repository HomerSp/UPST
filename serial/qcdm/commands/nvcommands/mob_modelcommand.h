#ifndef MOB_MODELCOMMAND
#define MOB_MODELCOMMAND

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class MobModelCommand : public NvCommand8Bit
                {
                public:
                    MobModelCommand(SerialDevice* device, bool read = true, uint8_t data = 0)
                        : NvCommand8Bit(device, read, NvItem::NV_MOB_MODEL_I, data)
                    {

                    }
                };
            }
        }
    }
}

#endif // MOB_MODELCOMMAND

