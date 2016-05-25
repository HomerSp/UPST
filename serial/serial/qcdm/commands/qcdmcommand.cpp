#include <QDebug>

#include "../../crcutils.h"
#include "qcdmcommand.h"

using namespace Serial::QCDM::Commands;

QcdmCommand::QcdmCommand(SerialDevice* device, QCDM::DiagCommands cmd, QByteArray data)
    : QcdmCommand(device)
{
    mCmds.append(new QcdmCommandItem(cmd, data));
}

QcdmCommand::QcdmCommand(SerialDevice* device, QcdmCommandItem *cmd)
    : QcdmCommand(device)
{
    mCmds.append(cmd);
}

QcdmCommand::QcdmCommand(SerialDevice* device, const QList<QcdmCommandItem*>& cmds)
    : SerialCommand(device)
{
    foreach(QcdmCommandItem *cmd, cmds) {
        mCmds.append(cmd);
    }
}

QcdmCommand::~QcdmCommand() {
    clearItems();
}

void QcdmCommand::execute() {
    execute(true);
}

void QcdmCommand::execute(bool obeyOffset) {
    execute(SerialCommand::device(), obeyOffset);
}

void QcdmCommand::execute(SerialDevice* device, bool obeyOffset) {
    if(device == nullptr) {
        return;
    }

    if(!device->communicator()->open()) {
        addResult(false);
        return;
    }

    QList<QByteArray> requests;
    bool r = getRequest(requests);

    if(!r || requests.size() == 0) {
        addResult(false);
        return;
    }

    for(int i = 0; i < requests.size(); i++) {
        QByteArray request = requests[i];

        QByteArray data;
        qDebug()<<"QcdmCommand writing"<<QString(request.toHex());
        if(!device->communicator()->write(request)) {
            addResult(false);
            return;
        }
        qDebug()<<"QcdmCommand reading...";
        if(!device->communicator()->read(data, timeout())) {
            addResult(false);
            return;
        }

        if(data.size() < 1) {
            qDebug()<<"QcdmCommand read less than 1 bytes";
            addResult(false);
            return;
        }

        qDebug()<<"QcdmCommand read"<<QString(data.toHex());

        uint8_t cmd = data[0];
        if(cmd != command(i)) {
            qDebug()<<"Diag error code"<<cmd<<"vs command"<<command(i);

            addResult(false, 0, 0, cmd);
            return;
        }

        if(!Serial::CRCUtils::checkAndRemoveCRC(data)) {
            qWarning()<<"Could not find a valid crc";

            addResult(false);
            return;
        }

        data.remove(0, 1);

        if(obeyOffset && offset() > 0) {
            data.remove(0, offset());
        }

        qDebug()<<"QcdmCommand read"<<QString(data.toHex());

        addResult(true, data, 0, cmd);
    }
}

bool QcdmCommand::getRequest(QList<QByteArray>& request) {
    request.clear();

    for(int i = 0; i < mCmds.size(); i++) {
        QByteArray data;
        data.append(mCmds.at(i)->cmd);
        data.append(mCmds.at(i)->data);

        Serial::CRCUtils::addCRC(data);
        request.append(data);
    }

    return true;
}
