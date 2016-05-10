#ifndef PASSWORDCOMMAND_H
#define PASSWORDCOMMAND_H

#include "qcdmcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            class PasswordCommand : public QcdmCommand
            {
            public:
                PasswordCommand(SerialDevice* device, QString password);
            };
        }
    }
}

#endif // PASSWORDCOMMAND_H
