#include "serialcommunicator.h"

#include <QDebug>

using namespace Serial;

SerialCommunicator::SerialCommunicator(const SerialDevice &device)
{
    mPort = new QSerialPort(device.port());
    setBaudRate(device.baudRate());
    setDataBits(device.dataBits());
    setParity(device.parity());
    setStopBits(device.stopBits());
}

SerialCommunicator::~SerialCommunicator() {
    close();

    delete mPort;
    mPort = nullptr;
}

bool SerialCommunicator::open() {
    if(mPort->isOpen()) {
        return true;
    }

    return mPort->open(QIODevice::ReadWrite);
}

bool SerialCommunicator::close() {
    if(!mPort->isOpen()) {
        return false;
    }

    mPort->flush();
    mPort->close();

    return true;
}

void SerialCommunicator::clear() {
    mPort->clear();
}

bool SerialCommunicator::read(QByteArray& data, int timeout) {
    if(!mPort->isOpen()) {
        return false;
    }

    if(timeout != 0) {
        qDebug()<<"SerialCommunicator read waitForReadyRead"<<timeout;
        if(!mPort->waitForReadyRead(timeout)) {
            qDebug()<<"SerialCommunicator read waitForReadyRead timeout";
            return false;
        }
    }

    qDebug()<<"SerialCommunicator read available"<<mPort->bytesAvailable();

    data = mPort->readAll();
    return data.size() > 0;
}

bool SerialCommunicator::write(const QByteArray& data) {
    if(!mPort->isOpen()) {
        return false;
    }

    qint64 written = mPort->write(data);
    return mPort->flush() && written == data.size();
}
