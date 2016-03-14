#ifndef PRLCOMMAND_H
#define PRLCOMMAND_H

#include "qcdmcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            class PRLCommand : public QcdmCommand
            {
            public:
                PRLCommand(SerialDevice* device, bool read = true, QByteArray data = 0);
            };
        }
    }
}

#endif // PRLCOMMAND_H
