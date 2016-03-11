#ifndef PROVISIONCOMMAND_H
#define PROVISIONCOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class ProvisionCommand : public NvCommand
                    {
                    public:
                        ProvisionCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue, QCDM::NvItem item, NvItemType itemType, uint8_t userProfIndex = 0);
                    };
                }
            }
        }
    }
}

#endif // PROVISIONCOMMAND_H
