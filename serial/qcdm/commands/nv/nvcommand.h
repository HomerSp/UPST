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
                class NvCommandItem : public QcdmCommandItem {
                public:
                    NvCommandItem(QCDM::DiagCommands cmd, QCDM::NvItem nvItem, QByteArray data = 0)
                        : QcdmCommandItem(cmd, data)
                    {
                        this->nvItem = nvItem;
                    }

                    QCDM::NvItem nvItem;
                };

                class NvCommand : public QcdmCommand
                {
                public:
                    NvCommand(SerialDevice* device, bool read, QCDM::NvItem nvItem, QByteArray data = 0);
                    NvCommand(SerialDevice* device, const QList<NvCommandItem*>& cmds);

                    virtual bool execute(QList<QByteArray>& result, uint16_t* errorCode = nullptr);
                    virtual bool execute(QByteArray& result, uint16_t* errorCode = nullptr);

                    QCDM::NvItem nvItem(int index = 0) {
                        QcdmCommandItem *item = QcdmCommand::item(index);
                        return static_cast<NvCommandItem*>(item)->nvItem;
                    }

                protected:
                    NvCommand(SerialDevice* device)
                        : QcdmCommand(device) {

                    }

                    virtual bool getRequest(QList<QByteArray>& request);
                };

                class NvCommand8Bit : public NvCommand
                {
                public:
                    NvCommand8Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint8_t data = 0);

                    bool execute(uint8_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommand16Bit : public NvCommand
                {
                public:
                    NvCommand16Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint16_t data = 0);

                    bool execute(uint16_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommand32Bit : public NvCommand
                {
                public:
                    NvCommand32Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint32_t data = 0);

                    bool execute(uint32_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommand64Bit : public NvCommand
                {
                public:
                    NvCommand64Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint64_t data = 0);

                    bool execute(uint64_t& result, uint16_t* errorCode = nullptr);

                };

                class NvCommandString : public NvCommand
                {
                public:
                    NvCommandString(SerialDevice* device, bool read, QCDM::NvItem item, QString data = "")
                        : NvCommand(device, read, item, data.toLatin1())
                    {

                    }

                    bool execute(QString& result, uint16_t* errorCode = nullptr);

                };
            }
        }
    }
}

#endif // NVCOMMAND_H
