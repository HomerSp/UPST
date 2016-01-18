#include <QDebug>

#include "../../crcutils.h"
#include "qcdmcommand.h"

using namespace Serial::QCDM::Commands;

QcdmCommand::QcdmCommand(SerialCommunicator* communicator, QCDM::DiagCommands cmd, QByteArray data)
    : SerialCommand(communicator),
     mCmd(cmd),
     mData(data)
{

}

bool QcdmCommand::execute(QByteArray& result, uint16_t* errorCode) {
    if(errorCode != nullptr) {
        *errorCode = 0;
    }

    result.clear();

    if(!communicator()->open()) {
        return false;
    }

    QByteArray request;
    getRequest(request);


    qDebug()<<"QcdmCommand writing"<<QString(request.toHex());
    if(!communicator()->write(request)) {
        return false;
    }
    qDebug()<<"QcdmCommand reading...";
    if(!communicator()->read(result, timeout())) {
        return false;
    }

    if(result.size() < 1) {
        qDebug()<<"QcdmCommand read less than 1 bytes";
        return false;
    }

    qDebug()<<"QcdmCommand read"<<QString(result.toHex());

    uint8_t cmd = result[0];
    if(cmd == Serial::QCDM::DiagCommands::DIAG_BAD_SPC_MODE_F) {
        if(errorCode != nullptr) {
            *errorCode = Serial::QCDM::DiagCommands::DIAG_BAD_SPC_MODE_F;
            return false;
        }
    }
    if(cmd != command()) {
        qDebug()<<"cmd"<<cmd<<"!="<<command();

        return false;
    }

    int crcSize = Serial::CRCUtils::verifyCRC(result);
    if(crcSize == 0) {
        return false;
    }

    result.remove(result.size() - crcSize, crcSize);
    result.remove(0, 1);

    qDebug()<<"QcdmCommand read"<<QString(result.toHex());

    return true;
}

bool QcdmCommand::getRequest(QByteArray& request) {
    request.clear();
    request.append(command());
    request.append(data());

    Serial::CRCUtils::addCRC(request);

    return true;
}
