#ifndef DEVICE_H
#define DEVICE_H

#include <QJsonObject>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "qcdm/nvitems.h"

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
        SerialProvisionStatusQueue,
        SerialProvisionStatusDoneRTRE,
        SerialProvisionStatusWrongSPC,
    };

    enum SerialProvisionError {
        SerialProvisionErrorNone = 0,
        SerialProvisionErrorDownload,
        SerialProvisionErrorData,
        SerialProvisionErrorNv,
        SerialProvisionErrorRemoved,
        SerialProvisionErrorRTRE,
    };

    enum SerialDeviceFlags {
        SerialDeviceFlagNone = 0x0,
        SerialDeviceFlagManualReboot = 0x1,
        SerialDeviceFlagMultiPort = 0x2,
        SerialDeviceFlagNoSPC = 0x4,
    };

    class SerialDevice : public QObject
    {
        Q_OBJECT
    public:
        SerialDevice(const QString& port, uint16_t vid, uint16_t pid);
        SerialDevice(const QSerialPortInfo& info);
        SerialDevice(const QJsonObject& obj);
        ~SerialDevice();

        SerialCommunicator* communicator();

        bool close();

        void addChild(SerialDevice* device);

        const QList<SerialDevice*> deviceChildren() const {
            return mChildren;
        }

        bool isAvailable();
        bool isProvisioning() const {
            // A child may still be showing as provisioning.
            foreach(Serial::SerialDevice* c, mChildren) {
                if(c->mProvisioning) {
                    return true;
                }
            }

            return mProvisioning;
        }
        bool isSameDevice(SerialDevice* device);
        bool isValid();

        bool childrenAvailable();

        bool canProvision();

        bool rtreSet() const {
            return mRTRESet;
        }

        void resetRTRE() {
            mRTRESet = false;
        }

        bool wrongSPC() const {
            return mWrongSPC;
        }

        void handleEventReport(const QByteArray& data);

        bool update();
        bool updateJson(const QJsonObject& obj);

        void updateFrom(Serial::SerialDevice* other);

        bool provision(const QString& userToken);

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

        void setProvisioning(bool f) {
            mProvisioning = f;
        }
        void setProvisionData(const QString& mdn, uint64_t min, const QString& spc = "") {
            mNewMdn = mdn;
            mNewMin = min;
            if(spc.size() > 0) {
                mSPC = spc;
            }
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

        const QJsonObject &guide() const {
            return mGuide;
        }

        bool flagManualReboot() const {
            return (mFlags & SerialDeviceFlagManualReboot) != 0;
        }
        bool flagMultiPort() const {
            return (mFlags & SerialDeviceFlagMultiPort) != 0;
        }
        bool flagNoSpc() const {
            return (mFlags & SerialDeviceFlagNoSPC) != 0;
        }

        const QString &mdn() const {
            return mMdn;
        }
        uint64_t min() const {
            return mMin;
        }
        const QString& newMdn() const {
            return mNewMdn;
        }
        uint64_t newMin() const {
            return mNewMin;
        }

        const QString& spc() const {
            return mSPC;
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

        QString id() const {
            return mID;
        }

        QString portStr() const;
        QString vidStr() const;
        QString pidStr() const;
        QString name() const;
        QString makeStr() const;
        QString modelStr() const;
        QString mdnStr() const;
        QString minStr() const;
        QString esnStr() const;
        QString meidStr() const;
        QString imeiStr() const;
        QString rtreStr() const;
        QString newMdnStr() const;
        QString newMinStr() const;

        void stopProvision() {
            mProvisionStop = true;
        }

    signals:
        void provisionProgressChanged(int status, int progress, int error = Serial::SerialProvisionErrorNone);

    private:
        static void updateFrom(Serial::SerialDevice* device, Serial::SerialDevice* other);

        bool provision(SerialProvisionData* data);
        bool provision(SerialDevice* device, SerialProvisionData* data, SerialCommand* cmd);

        bool sendSPC(SerialDevice* device);

        SerialCommunicator* mCommunicator;

        QList<SerialDevice*> mChildren;

        QString mPort;
        uint16_t mVid;
        uint16_t mPid;

        QSerialPort::BaudRate mBaudRate;
        QSerialPort::DataBits mDataBits;
        QSerialPort::Parity mParity;
        QSerialPort::StopBits mStopBits;

        QString mID;
        QString mMake;
        QString mModel;
        QString mCodename;
        SerialDeviceType mType;
        uint32_t mFlags;
        QJsonObject mGuide;

        QString mMdn;
        uint64_t mMin;
        uint32_t mESN;
        uint64_t mIMEI;
        uint64_t mMEID;
        Serial::QCDM::RTREMode mRTRE;

        bool mProvisioning;
        bool mProvisionStop;
        bool mRTRESet;
        QString mNewMdn;
        uint64_t mNewMin;
        QString mSPC;
        bool mWrongSPC;
    };
}

#endif // DEVICE_H
