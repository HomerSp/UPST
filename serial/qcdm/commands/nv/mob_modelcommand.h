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
                    MobModelCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QByteArray data = 0)
                        : NvCommand8Bit(communicator, cmd, NvItem::NV_MOB_MODEL_I, data)
                    {

                    }
                };
            }
        }
    }
}

#endif // MOB_MODELCOMMAND

