#ifndef GENERICNAICOMMAND_H
#define GENERICNAICOMMAND_H

#include "../nvcommand.h"
#include "../../../../serialprovisiondata.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class GenericNaiCommand : public NvCommand
                    {
                    public:
                        GenericNaiCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue, QCDM::NvItem item, NvItemType itemType, SerialProvisionData::UserType userType, uint8_t userProfIndex = 0);
                    };
                }
            }
        }
    }
}

#endif // GENERICNAICOMMAND_H
