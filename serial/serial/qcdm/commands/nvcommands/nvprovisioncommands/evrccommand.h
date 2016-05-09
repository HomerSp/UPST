#ifndef EVRCCOMMAND_H
#define EVRCCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class EvrcCommand : public NvCommand
                    {
                    public:
                        EvrcCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr);
                    };
                }
            }
        }
    }
}

#endif // EVRCCOMMAND_H
