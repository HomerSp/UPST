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

                    NvCommandItem(NvCommandItem* item)
                        : NvCommandItem(item->cmd, item->nvItem, item->data)
                    {

                    }

                    QCDM::NvItem nvItem;
                };

                class NvCommand : public QcdmCommand
                {
                public:
                    NvCommand(SerialDevice* device, bool read, QCDM::NvItem nvItem, QByteArray data = 0);
                    NvCommand(SerialDevice* device, const QList<NvCommandItem*>& cmds);

                    virtual void execute();

                    QCDM::NvItem nvItem(int index = 0) {
                        QcdmCommandItem *item = QcdmCommand::item(index);
                        return static_cast<NvCommandItem*>(item)->nvItem;
                    }

                protected:
                    NvCommand(SerialDevice* device)
                        : QcdmCommand(device) {

                    }

                    virtual bool getRequest(QList<QByteArray>& request);

                    uint64_t getDecodedValue(uint64_t val, uint8_t width);
                    uint64_t getEncodedValue(uint64_t val, uint8_t width);
                };

                class NvCommand8Bit : public NvCommand
                {
                public:
                    NvCommand8Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint8_t data = 0);

                    virtual void execute();

                    uint8_t resultData() {
                        return result()->data().toChar().toLatin1();                    }

                };

                class NvCommand16Bit : public NvCommand
                {
                public:
                    NvCommand16Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint16_t data = 0);

                    virtual void execute();

                    uint16_t resultData() {
                        return result()->data().toUInt();
                    }

                };

                class NvCommand32Bit : public NvCommand
                {
                public:
                    NvCommand32Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint32_t data = 0);

                    virtual void execute();

                    uint32_t resultData() {
                        return result()->data().toUInt();
                    }

                };

                class NvCommand64Bit : public NvCommand
                {
                public:
                    NvCommand64Bit(SerialDevice* device, bool read, QCDM::NvItem item, uint64_t data = 0);

                    virtual void execute();

                    uint64_t resultData() {
                        return result()->data().toULongLong();
                    }
                };

                class NvCommandString : public NvCommand
                {
                public:
                    NvCommandString(SerialDevice* device, bool read, QCDM::NvItem item, QString data = "")
                        : NvCommand(device, read, item, data.toLatin1())
                    {

                    }

                    virtual void execute();

                    QString resultData() {
                        return result()->data().toString();
                    }
                };
            }
        }
    }
}

#endif // NVCOMMAND_H
