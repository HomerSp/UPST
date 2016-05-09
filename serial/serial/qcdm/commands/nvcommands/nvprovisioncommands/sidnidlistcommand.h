#ifndef SIDNIDLISTCOMMAND_H
#define SIDNIDLISTCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class SidNidListCommand : public NvCommand
                    {
                    public:
                        SidNidListCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr);
                    };
                }
            }
        }
    }
}

#endif // SIDNIDLISTCOMMAND_H
