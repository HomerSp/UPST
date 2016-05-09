#ifndef MNCIMSICOMMAND_H
#define MNCIMSICOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class MncImsiCommand : public NvCommand
                    {
                    public:
                        MncImsiCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr);
                    };
                }
            }
        }
    }
}

#endif // MNCIMSICOMMAND_H
