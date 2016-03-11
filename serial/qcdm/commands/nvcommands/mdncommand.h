#ifndef MDNCOMMAND_H
#define MDNCOMMAND_H

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class MDNCommand : public NvCommandString
                {
                public:
                    MDNCommand(SerialDevice* device, bool read = true, QString data = 0);
                };
            }
        }
    }
}

#endif // MDNCOMMAND_H
