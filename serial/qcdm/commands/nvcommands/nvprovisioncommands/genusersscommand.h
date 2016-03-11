#ifndef GENUSERSSCOMMAND_H
#define GENUSERSSCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class GenUserSSCommand : public NvCommand
                    {
                    public:
                        GenUserSSCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr);
                    };
                }
            }
        }
    }
}

#endif // GENUSERSSCOMMAND_H
