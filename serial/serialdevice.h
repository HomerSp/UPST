#ifndef DEVICE_H
#define DEVICE_H

#include <QJsonObject>
#include <QSerialPort>
#include <QSerialPortInfo>

namespace Serial {
    class SerialCommunicator;
    class SerialCommand;
    class SerialProvisionData;

    enum SerialDeviceType {
        SerialDeviceTypeUnknown = 0,
        SerialDeviceTypeSmartphone,
        SerialDeviceTypeFeaturePhone,
        SerialDeviceTypeTablet,
        SerialDeviceTypeMifi,
    };

    enum SerialProvisionStatus {
        SerialProvisionStatusIdle = 0,
        SerialProvisionStatusProgress,
        SerialProvisionStatusDone,
        SerialProvisionStatusError,
    };

    enum SerialProvisionError {
        SerialProvisionErrorNone = 0,
        SerialProvisionErrorDownload,
        SerialProvisionErrorData,
        SerialProvisionErrorNv,
    };

    class SerialDevice : public QObject
    {
        Q_OBJECT
    public:
        SerialDevice(const QString& port, uint16_t vid, uint16_t pid);
        SerialDevice(const QSerialPortInfo& info);
        ~SerialDevice();

        SerialCommunicator* communicator();

        void addChild(SerialDevice* device);

        const QList<SerialDevice*> deviceChildren() const {
            return mChildren;
        }

        bool isSameDevice(SerialDevice* device);
        bool isValid();

        bool update();

        bool updateJson(const QJsonObject& obj);

        bool provision();

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

        void setProvisionData(const QString& mdn, uint64_t min) {
            mNewMdn = mdn;
            mNewMin = min;
        }

        bool operator==(const SerialDevice& other);
        bool operator==(const QString& port);

        const QString& port() const {
            return mPort;
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

        uint16_t vid() const {
            return mVid;
        }

        uint16_t pid() const {
            return mPid;
        }

        const QString& make() const {
            return mMake;
        }
        const QString& model() const {
            return mModel;
        }
        const QString& codename() const {
            return mCodename;
        }

        SerialDeviceType type() const {
            return mType;
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

        QString portStr() const {
            if(mChildren.size() == 0) {
                return mPort;
            }

            QString ret = mPort;
            foreach(SerialDevice* d, mChildren) {
                ret += ", " + d->port();
            }

            return ret;
        }

        QString vidStr() const {
            return QString("%1").arg(mVid, 4, 16, QChar('0')).toUpper();
        }
        QString pidStr() const {
            return QString("%1").arg(mPid, 4, 16, QChar('0')).toUpper();
        }

        QString name() const {
            QString ret = "";
            if(mMake.length() != 0) {
                ret += mMake;
            }
            if(mModel.length() != 0) {
                if(ret.length() > 0) {
                    ret += " ";
                }

                ret += mModel;
            }

            return ret;
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

            return QString("%1").arg(mESN, 8, 16, QChar('0')).toUpper();
        }
        QString meidStr() const {
            if(mMEID == 0) {
                return "-";
            }

            return QString("%1").arg(mMEID, 14, 16, QChar('0')).toUpper();
        }
        QString imeiStr() const {
            if(mIMEI == 0) {
                return "-";
            }

            return QString("%1").arg(mIMEI, 14, 16, QChar('0')).toUpper();
        }

        QString newMdnStr() const {
            if(mNewMdn.length() == 0) {
                return mMdn;
            }

            return mNewMdn;
        }
        QString newMinStr() const {
            if(mNewMin == 0) {
                QString ret = minStr();
                if(ret == "-") {
                    return "";
                }

                return ret;
            }

            return QString("%1").arg(mNewMin, 10, 10, QChar('0'));
        }

    signals:
        void provisionProgressChanged(int status, int progress, int error = Serial::SerialProvisionErrorNone);

    private:
        bool provision(SerialProvisionData* data);
        bool provision(SerialDevice* device, SerialProvisionData* data, SerialCommand* cmd);

        SerialCommunicator* mCommunicator;

        QList<SerialDevice*> mChildren;

        QString mPort;
        uint16_t mVid;
        uint16_t mPid;

        QSerialPort::BaudRate mBaudRate;
        QSerialPort::DataBits mDataBits;
        QSerialPort::Parity mParity;
        QSerialPort::StopBits mStopBits;

        QString mMake;
        QString mModel;
        QString mCodename;
        SerialDeviceType mType;

        QString mMdn;
        uint64_t mMin;
        uint32_t mESN;
        uint64_t mIMEI;
        uint64_t mMEID;

        QString mNewMdn;
        uint64_t mNewMin;
    };
}

#endif // DEVICE_H
