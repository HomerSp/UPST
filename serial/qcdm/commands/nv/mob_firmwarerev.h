#ifndef MOB_FIRMWAREREV
#define MOB_FIRMWAREREV

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class MobFirmwareRevCommand : public NvCommand16Bit
                {
                public:
                    MobFirmwareRevCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QByteArray data = 0)
                        : NvCommand16Bit(communicator, cmd, NvItem::NV_MOB_FIRM_REV_I, data)
                    {

                    }
                };
            }
        }
    }
}

#endif // MOB_FIRMWAREREV

