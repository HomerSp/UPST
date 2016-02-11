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

        const QString& description() const {
            return mDescription;
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

        const QString &mdn() const {
            return mMdn;
        }

        uint64_t min() const {
            return mMin;
        }
        uint32_t esn() const {
            return mESN;
        }
        uint64_t imei() const {
            return mIMEI;
        }
        uint64_t meid() const {
            return mMEID;
        }

        QString makeStr() const {
            if(mMake.length() == 0) {
                return "-";
            }

            return mMake;
        }
        QString modelStr() const {
            if(mModel.length() == 0) {
                return "-";
            }

            return mModel;
        }
        QString mdnStr() const {
            if(mMdn.length() == 0) {
                return "-";
            }

            return mMdn;
        }
        QString minStr() const {
            if(mMin == 0) {
                return "-";
            }

            return QString("%1").arg(mMin, 10, 10, QChar('0'));
        }
        QString esnStr() const {
            if(mESN == 0) {
                return "-";
            }

            return QString("%1").arg(mESN, 8, 16, QChar('0'));
        }
        QString meidStr() const {
            if(mMEID == 0) {
                return "-";
            }

            return QString("%1").arg(mMEID, 14, 16, QChar('0'));
        }
        QString imeiStr() const {
            if(mIMEI == 0) {
                return "-";
            }

            return QString("%1").arg(mIMEI, 14, 16, QChar('0'));
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

        QString mMake;
        QString mModel;
        QString mMdn;
        uint64_t mMin;
        uint32_t mESN;
        uint64_t mIMEI;
        uint64_t mMEID;
    };
}

#endif // DEVICE_H
