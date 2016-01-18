#ifndef NVCOMMAND_H
#define NVCOMMAND_H

#include <QByteArray>

#include "../../diag.h"
#include "../../nvitems.h"
#include "../../../crcutils.h"
#include "../../../serialcommunicator.h"
#include "../qcdmcommand.h"

namespace Serial {
    class SerialCommunicator;

    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class NvCommand : public QcdmCommand
                {
                public:
                    NvCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QCDM::NvItem item, QByteArray data = 0);

                    virtual bool execute(QByteArray& result, uint16_t* errorCode = nullptr);

                protected:
                    bool getRequest(QByteArray& request);

                private:
                    QCDM::NvItem mItem;
                };

                class NvCommand8Bit : public NvCommand
                {
                public:
                    NvCommand8Bit(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QCDM::NvItem item, QByteArray data = 0)
                        : NvCommand(communicator, cmd, item, data)
                    {

                    }

                    bool execute(uint8_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommand16Bit : public NvCommand
                {
                public:
                    NvCommand16Bit(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QCDM::NvItem item, QByteArray data = 0)
                        : NvCommand(communicator, cmd, item, data)
                    {

                    }

                    bool execute(uint16_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommand32Bit : public NvCommand
                {
                public:
                    NvCommand32Bit(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QCDM::NvItem item, QByteArray data = 0)
                        : NvCommand(communicator, cmd, item, data)
                    {

                    }

                    bool execute(uint32_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommand64Bit : public NvCommand
                {
                public:
                    NvCommand64Bit(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QCDM::NvItem item, QByteArray data = 0)
                        : NvCommand(communicator, cmd, item, data)
                    {

                    }

                    bool execute(uint64_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommandString : public NvCommand
                {
                public:
                    NvCommandString(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QCDM::NvItem item, QByteArray data = 0)
                        : NvCommand(communicator, cmd, item, data)
                    {

                    }

                    bool execute(QString& result, uint16_t* errorCode = nullptr);

                };
            }
        }
    }
}

#endif // NVCOMMAND_H
