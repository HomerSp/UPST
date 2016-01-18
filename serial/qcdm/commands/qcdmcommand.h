#ifndef QCDMCOMMAND_H
#define QCDMCOMMAND_H

#include <QByteArray>
#include <QString>

#include "../../serialcommand.h"
#include "../../serialcommunicator.h"
#include "../diag.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            class QcdmCommand : public SerialCommand
            {
            public:
                QcdmCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QByteArray data = 0);

                virtual bool execute(QByteArray& result, uint16_t* errorCode = nullptr);

            protected:
                const QCDM::DiagCommands &command() {
                    return mCmd;
                }

                const QByteArray &data() {
                    return mData;
                }

                void setData(const QByteArray& data) {
                    mData = data;
                }

                virtual bool getRequest(QByteArray& request);

            private:
                QCDM::DiagCommands mCmd;
                QByteArray mData;
            };
        }
    }
}

#endif // QCDMCOMMAND_H
