#ifndef HDRANUSERCOMMAND_H
#define HDRANUSERCOMMAND_H

#include "genericnaicommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class HDRAnUserCommand : public GenericNaiCommand
                    {
                    public:
                        HDRAnUserCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue, NvItemType itemType, SerialProvisionData::UserType userType, uint8_t userProfIndex = 0);
                    };
                }
            }
        }
    }
}

#endif // HDRANUSERCOMMAND_H
