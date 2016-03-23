#ifndef MCCIMSICOMMAND_H
#define MCCIMSICOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class MccImsiCommand : public NvCommand
                    {
                    public:
                        MccImsiCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr);
                    };
                }
            }
        }
    }
}

#endif // MCCIMSICOMMAND_H
