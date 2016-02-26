#ifndef SERIALDEVICEWORKER_H
#define SERIALDEVICEWORKER_H

#include <QAtomicInteger>
#include <QMutex>
#include <QWaitCondition>
#include <QSerialPortInfo>

#include "../serial/serialdevice.h"
#include "../serial/serialcommand.h"

namespace UI {
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

        void addNewDevice(const QSerialPortInfo &info);
        void addCommand(SerialCommandItem* cmd);

        void removeDevice(Serial::SerialDevice* device);

        void stop();

    public slots:
        void process();

    signals:
        void finished();

        void deviceAdd(Serial::SerialDevice* device);

        void statusChange(const QString& status);

    private:
        void processDeviceRemovals();
        void processNewDevice();
        void processCommand();

        QMutex mWakeMutex;
        QWaitCondition mWaitCondition;

        QMutex mRunningMutex;
        QAtomicInteger<bool> mRunning;

        QMutex mPortsMutex;
        QList<QSerialPortInfo*> mPorts;

        QMutex mDeviceRemoveMutex;
        QList<Serial::SerialDevice*> mDeviceRemove;

        QMutex mCommandsMutex;
        QList<SerialCommandItem*> mCommands;
    };
}

#endif // SERIALDEVICEWORKER_H
