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

                    void execute();
                };
            }
        }
    }
}

#endif // IMEICOMMAND

