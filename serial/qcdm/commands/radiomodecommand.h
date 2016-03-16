#ifndef RADIOMODECOMMAND_H
#define RADIOMODECOMMAND_H

#include "qcdmcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            class RadioModeCommand : public QcdmCommand
            {
            public:
                RadioModeCommand(SerialDevice* device, Serial::QCDM::Mode mode);
            };
        }
    }
}

#endif // RADIOMODECOMMAND_H
