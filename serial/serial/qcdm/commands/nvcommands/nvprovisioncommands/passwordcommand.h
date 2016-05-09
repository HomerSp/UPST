#ifndef PASSWORDCOMMAND_H
#define PASSWORDCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class PasswordCommand : public NvCommand
                    {
                    public:
                        PasswordCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr);
                    };
                }
            }
        }
    }
}

#endif // PASSWORDCOMMAND_H
