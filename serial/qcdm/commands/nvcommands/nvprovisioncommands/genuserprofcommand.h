#ifndef GENUSERPROFCOMMAND_H
#define GENUSERPROFCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class GenUserProfCommand : public NvCommand
                    {
                    public:
                        GenUserProfCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr, QString userNai = "");
                    };
                }
            }
        }
    }
}

#endif // GENUSERPROFCOMMAND_H
