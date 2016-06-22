#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <QSerialPort>

#include "serialdevice.h"

namespace Serial {
    class SerialCommunicator : public QObject
    {
        Q_OBJECT
    public:
        SerialCommunicator(const SerialDevice &device);
        ~SerialCommunicator();

        bool open();
        bool close();

        bool isOpen() {
            return mPort != nullptr && mPort->isOpen();
        }

        void clear();

        bool read(QByteArray& data, int timeout, bool& timedout);
        bool write(const QByteArray& data);

        const QString port() const {
            return mPort->portName();
        }

        inline void setPort(const QString& port) {
            mPort->setPortName(port);
        }

        inline void setBaudRate(QSerialPort::BaudRate baudRate) {
            mPort->setBaudRate(baudRate);
        }

        inline void setDataBits(QSerialPort::DataBits dataBits) {
            mPort->setDataBits(dataBits);
        }

        inline void setParity(QSerialPort::Parity parity) {
            mPort->setParity(parity);
        }

        inline void setStopBits(QSerialPort::StopBits stopBits) {
            mPort->setStopBits(stopBits);
        }

    private:
        QSerialPort *mPort;
    };
}

#endif // SERIALCOMMUNICATOR_H
