#ifndef QCDMCOMMAND_H
#define QCDMCOMMAND_H

#include <QByteArray>
#include <QString>
#include <QPair>

#include "../../serialcommand.h"
#include "../../serialcommunicator.h"
#include "../diag.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            class QcdmCommandItem {
            public:
                QcdmCommandItem(QCDM::DiagCommands cmd, QByteArray data) {
                    this->cmd = cmd;
                    this->data = data;
                }

                QcdmCommandItem(const QcdmCommandItem &item)
                    : QcdmCommandItem(item.cmd, item.data)
                {
                }

                QCDM::DiagCommands cmd;
                QByteArray data;
            };

            class QcdmCommand : public SerialCommand
            {
            public:
                QcdmCommand(SerialDevice* device, QCDM::DiagCommands cmd, QByteArray data = 0);
                QcdmCommand(SerialDevice* device, const QList<QcdmCommandItem *> &cmds);
                ~QcdmCommand();

                int count() {
                    return mCmds.size();
                }

                virtual void execute();

            protected:
                QcdmCommand(SerialDevice* device) : SerialCommand(device) {

                }
                QcdmCommand(SerialDevice* device, QcdmCommandItem *cmd);

                void addItem(QcdmCommandItem* item) {
                    mCmds.append(item);
                }

                QcdmCommandItem* item(int i = 0) {
                    return mCmds[i];
                }

                const QCDM::DiagCommands &command(int i = 0) {
                    return mCmds[i]->cmd;
                }

                const QByteArray &data(int i = 0) {
                    return mCmds[i]->data;
                }

                void setData(const QByteArray& data, int i = 0) {
                    mCmds[i]->data = data;
                }

                virtual bool getRequest(QList<QByteArray>& request);

            private:
                QList<QcdmCommandItem *> mCmds;

            };
        }
    }
}

#endif // QCDMCOMMAND_H
