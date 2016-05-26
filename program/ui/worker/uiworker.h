#ifndef UIWORKER_H
#define UIWORKER_H

#include <QMutex>
#include <QString>
#include <QWaitCondition>

#include "../serial/serial/serialdevice.h"
#include "../serial/serial/serialdeviceconfig.h"

namespace UI {
    namespace Worker {
        struct LoginItem {
            QString username;
            QString password;
            QString token;
        };

        struct ProvisionTrackingItem {
            Serial::SerialDevice* device;
            bool error;
            QString log;
        };

        class UIWorker : public QObject
        {
            Q_OBJECT
        public:
            UIWorker(bool updateFailed);
            ~UIWorker();

            void addDevicesListUpdate();

            void addDeviceProvisionTracking(Serial::SerialDevice* device, bool error, const QString& log = QString());

            void addLogin(const QString& username, const QString& password);
            void addLoginCheck(const QString& token);

            void start();
            void stop();

        public slots:
            void process();

        signals:
            void finished();

            void devicesListChanged(bool success, Serial::SerialDeviceConfig* config = nullptr);

            void loginStatus(bool success);

            void updateCheck();
            void updateAvailable(const QString& updaterDir, const QString& id, const QString& version, const QDateTime& updateTime);

            void statusChange(const QString& status);

        private:
            enum WorkType {
                WorkTypeDevicesListUpdate,
                WorkTypeDeviceProvisionTracking,
                WorkTypeLogin,
            };

            void processDevicesListUpdate();
            bool processDeviceProvisionTracking(ProvisionTrackingItem* item);
            void processLogin(LoginItem* item);

            bool processLoginCheckUpdate(const QString& token);

            QThread* mThread;

            QMutex mWakeMutex;
            QWaitCondition mWaitCondition;

            QMutex mRunningMutex;
            QMutex mStoppedMutex;
            QAtomicInteger<bool> mRunning;

            QMutex mWorkMutex;
            QList<QPair<WorkType, void*> > mWorkItems;

            bool mUpdateFailed;
        };
    }
}

#endif // UIWORKER_H
