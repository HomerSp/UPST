#ifndef PASSWORDCOMMAND_H
#define PASSWORDCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class AuthPasswordCommand : public NvCommand
                    {
                    public:
                        AuthPasswordCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr);
                    };
                }
            }
        }
    }
}

#endif // PASSWORDCOMMAND_H
