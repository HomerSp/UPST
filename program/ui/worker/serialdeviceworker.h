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
    namespace Worker {
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

            void addDeviceCheck(const QSerialPortInfo &info);
            void addDeviceRemove(Serial::SerialDevice* device);
            void addDeviceClose(Serial::SerialDevice* device);

            void addDeviceProvision(Serial::SerialDevice* device);

            void addCommand(SerialCommandItem* cmd);

            void setDeviceConfig(Serial::SerialDeviceConfig* config);

            void start();
            void stop();

        public slots:
            void process();

            void deviceProvisionProgressChanged(int status, int progress, int error = Serial::SerialProvisionErrorNone);

        signals:
            void finished();

            void deviceAdd(Serial::SerialDevice* device);
            void deviceAddReschedule(QString port);
            void deviceClose(Serial::SerialDevice* device);

            void provisionProgressChanged(Serial::SerialDevice* device, int status, int progress, int error);
            void provisionSPCFailed(Serial::SerialDevice* device);

            void statusChange(const QString& status);

        private:
            enum WorkType {
                WorkTypeDeviceCheck,
                WorkTypeDeviceRemove,
                WorkTypeDeviceClose,
                WorkTypeCommand,
                WorkTypeDeviceProvision,
            };

            void processDeviceCheck(QSerialPortInfo* portInfo);
            void processDeviceRemove(Serial::SerialDevice* device, bool close = false);
            void processDeviceProvision(Serial::SerialDevice* device);
            void processCommand(SerialCommandItem* command);

            QThread* mThread;

            QMutex mWakeMutex;
            QWaitCondition mWaitCondition;

            QMutex mRunningMutex;
            QMutex mStoppedMutex;
            QAtomicInteger<bool> mRunning;

            QMutex mWorkMutex;
            QList<QPair<WorkType, void*> > mWorkItems;

            Serial::SerialDeviceConfig* mDeviceConfig;
        };
    }
}

#endif // SERIALDEVICEWORKER_H
