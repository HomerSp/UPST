#ifndef SUBSYSCOMMAND_H
#define SUBSYSCOMMAND_H

#include <QByteArray>
#include "../../qcdm/diag.h"
#include "../commands/qcdmcommand.h"

namespace Serial {
    class SerialDevice;

    namespace QCDM {
        namespace Commands {
            class SubSysCommand : public QcdmCommand
            {
            public:
                SubSysCommand(SerialDevice* device, DiagSubSysID id, uint16_t code, QByteArray data = 0);

                virtual void execute();

            private:
                DiagSubSysID mID;
                uint16_t mCode;
            };
        }
    }
}

#endif // SUBSYSCOMMAND_H
