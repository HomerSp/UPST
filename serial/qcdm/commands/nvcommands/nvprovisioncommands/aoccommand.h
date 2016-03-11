#ifndef AOCCOMMAND_H
#define AOCCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class AOCCommand : public NvCommand
                    {
                    public:
                        AOCCommand(SerialDevice* device, bool read = true);
                    };
                }
            }
        }
    }
}

#endif // AOCCOMMAND_H
