#ifndef SERIALDEVICEWORKER_H
#define SERIALDEVICEWORKER_H

#include <QAtomicInteger>
#include <QMutex>
#include <QWaitCondition>
#include <QSerialPortInfo>
#include <QPair>
#include <QTemporaryDir>

#include "serial/serialdevice.h"
#include "serial/serialdeviceconfig.h"
#include "serial/serialcommand.h"

namespace UI {
    struct LoginItem {
        QString username;
        QString password;
        QString token;
    };

    class SerialCommandItem : public QObject {
        Q_OBJECT
    public:
        SerialCommandItem(Serial::SerialDevice* device);
        ~SerialCommandItem();

        void addItem(Serial::SerialCommand* cmd);

        void process();

        Serial::SerialDevice* device() {
            return mDevice;
        }

        QList<Serial::SerialCommand*> &cmds() {
            return mCmds;
        }

    signals:
        void finished();

    private:
        Serial::SerialDevice* mDevice;
        QList<Serial::SerialCommand*> mCmds;
    };

    class SerialDeviceWorker: public QObject
    {
        Q_OBJECT
    public:
        SerialDeviceWorker();
        ~SerialDeviceWorker();

        void addDevicesListUpdate();

        void addDeviceCheck(const QSerialPortInfo &info);
        void addDeviceRemove(Serial::SerialDevice* device);

        void addDeviceProvision(Serial::SerialDevice* device);

        void addCommand(SerialCommandItem* cmd);

        void addLogin(const QString& username, const QString& password);
        void addLoginCheck(const QString& token);

        void stop();

    public slots:
        void process();

        void deviceProvisionProgressChanged(int status, int progress, int error = Serial::SerialProvisionErrorNone);

    signals:
        void finished();

        void devicesListChanged(bool success);
        void deviceAdd(Serial::SerialDevice* device);

        void provisionProgressChanged(Serial::SerialDevice* device, int status, int progress, int error);
        void provisionSPCFailed(Serial::SerialDevice* device);

        void loginStatus(bool success, const QString& token);

        void updateCheck();
        void updateAvailable(const QString& updaterDir, const QString& token, const QString& id, const QString& version, const QDateTime& updateTime);

        void statusChange(const QString& status);

    private:
        enum WorkType {
            WorkTypeDevicesListUpdate,
            WorkTypeDeviceCheck,
            WorkTypeDeviceRemove,
            WorkTypeCommand,
            WorkTypeDeviceProvision,
            WorkTypeLogin,
        };

        void processDeviceRemove(Serial::SerialDevice* device);
        void processDevicesListUpdate();
        void processDeviceCheck(QSerialPortInfo* portInfo);
        void processDeviceProvision(Serial::SerialDevice* device);
        void processCommand(SerialCommandItem* command);
        void processLogin(LoginItem* item);

        bool processLoginCheckUpdate(const QString& token);

        Serial::SerialDeviceConfig* mDeviceConfig;

        QMutex mWakeMutex;
        QWaitCondition mWaitCondition;

        QMutex mRunningMutex;
        QMutex mStoppedMutex;
        QAtomicInteger<bool> mRunning;

        QMutex mWorkMutex;
        QList<QPair<WorkType, void*> > mWorkItems;
    };
}

#endif // SERIALDEVICEWORKER_H
