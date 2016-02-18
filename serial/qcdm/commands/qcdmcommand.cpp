#include <QDebug>

#include "../../crcutils.h"
#include "qcdmcommand.h"

using namespace Serial::QCDM::Commands;

QcdmCommand::QcdmCommand(SerialDevice* device, QCDM::DiagCommands cmd, QByteArray data)
    : SerialCommand(device)
{
    mCmds.append(new QcdmCommandItem(cmd, data));
}

QcdmCommand::QcdmCommand(SerialDevice* device, QcdmCommandItem *cmd)
    : SerialCommand(device)
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
    foreach(QcdmCommandItem* cmd, mCmds) {
        delete cmd;
    }
}

bool QcdmCommand::execute(QList<QByteArray>& result, uint16_t* errorCode) {
    if(errorCode != nullptr) {
        *errorCode = 0;
    }

    result.clear();

    if(!communicator()->open()) {
        return false;
    }

    QList<QByteArray> requests;
    getRequest(requests);


    for(int i = 0; i < requests.size(); i++) {
        QByteArray request = requests[i];

        QByteArray data;
        qDebug()<<"QcdmCommand writing"<<QString(request.toHex());
        if(!communicator()->write(request)) {
            return false;
        }
        qDebug()<<"QcdmCommand reading...";
        if(!communicator()->read(data, timeout())) {
            return false;
        }

        if(data.size() < 1) {
            qDebug()<<"QcdmCommand read less than 1 bytes";
            return false;
        }

        qDebug()<<"QcdmCommand read"<<QString(data.toHex());

        uint8_t cmd = data[0];
        if(cmd == Serial::QCDM::DiagCommands::DIAG_BAD_SPC_MODE_F) {
            if(errorCode != nullptr) {
                *errorCode = Serial::QCDM::DiagCommands::DIAG_BAD_SPC_MODE_F;
                return false;
            }
        }
        if(cmd != command(i)) {
            qDebug()<<"cmd"<<cmd<<"!="<<command(i);

            return false;
        }

        int crcSize = Serial::CRCUtils::verifyCRC(data);
        if(crcSize == 0) {
            return false;
        }

        data.remove(data.size() - crcSize, crcSize);
        data.remove(0, 1);

        qDebug()<<"QcdmCommand read"<<QString(data.toHex());

        result.append(data);
    }

    return true;
}

bool QcdmCommand::execute(QByteArray& result, uint16_t* errorCode) {
    QList<QByteArray> ret;
    if(!execute(ret, errorCode)) {
        return false;
    }

    result = ret.at(0);
    return true;
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
