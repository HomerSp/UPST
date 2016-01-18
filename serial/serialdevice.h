#ifndef DEVICE_H
#define DEVICE_H

#include <QSerialPort>
#include <QSerialPortInfo>

namespace Serial {
    class SerialCommunicator;

    class SerialDevice : public QObject
    {
        Q_OBJECT
    public:
        SerialDevice(const QString& port, uint16_t vid, uint16_t pid, const QString& manufacturer, const QString& description, bool isAndroid);
        SerialDevice(const QSerialPortInfo& info, bool isAndroid);
        ~SerialDevice();

        SerialCommunicator* communicator();

        bool isValid();

        bool update();

        void setBaudRate(QSerialPort::BaudRate baudRate) {
            mBaudRate = baudRate;
        }
        void setDataBits(QSerialPort::DataBits dataBits) {
            mDataBits = dataBits;
        }
        void setParity(QSerialPort::Parity parity) {
            mParity = parity;
        }
        void setStopBits(QSerialPort::StopBits stopBits) {
            mStopBits = stopBits;
        }

        bool operator==(const SerialDevice& other);
        bool operator==(const QString& port);

        const QString& port() const {
            return mPort;
        }

        bool isAndroid() const {
            return mIsAndroid;
        }

        QSerialPort::BaudRate baudRate() const {
            return mBaudRate;
        }
        QSerialPort::DataBits dataBits() const {
            return mDataBits;
        }
        QSerialPort::Parity parity() const {
           return  mParity;
        }
        QSerialPort::StopBits stopBits() const {
            return mStopBits;
        }

        QString mdn() const {
            return mMdn;
        }

    private:
        SerialCommunicator* mCommunicator;

        QString mPort;
        uint16_t mVid;
        uint16_t mPid;
        QString mManufacturer;
        QString mDescription;
        bool mIsAndroid;

        QSerialPort::BaudRate mBaudRate;
        QSerialPort::DataBits mDataBits;
        QSerialPort::Parity mParity;
        QSerialPort::StopBits mStopBits;

        QString mMdn;
    };
}

#endif // DEVICE_H
