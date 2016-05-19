#ifndef UI_H
#define UI_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTemporaryDir>
#include <QTimer>
#include <QSet>

#include "connecteddevicesmodel.h"
#include "serialdeviceworker.h"
#include "loghandler.h"

#include "serial/serialdevice.h"

namespace UI {
    class UISection;

    class WebDownloader: public QObject {
        Q_OBJECT
    public:
        Q_INVOKABLE QString download(const QString& url) const;
    };

    class MainUI : public QObject
    {
        Q_OBJECT
    public:
        MainUI(const QGuiApplication& app, Log::LogHandler* logHandler);
        ~MainUI();

        QQmlApplicationEngine* engine() {
            return mEngine;
        }

        QList<Serial::SerialDevice*> &devices() {
            return mDevices;
        }

        SerialDeviceWorker* worker() {
            return mWorker;
        }

        ConnectedDevicesModel* devicesModel() {
            return mDevicesModel;
        }

        int currentIndex() {
            return mCurrentIndex;
        }

    signals:
        void deviceChanged(Serial::SerialDevice* device, bool added);
        void deviceUpdate(Serial::SerialDevice* device);

        void loggedIn();
        void loggedOut();

    public slots:
        void devicesChanged();

        void devicesListChanged(bool success);
        void deviceAdd(Serial::SerialDevice* device);
        void deviceAddReschedule(QString port);
        void deviceClose(Serial::SerialDevice* device);
        void deviceRemove(const QString& port);

        void setStatus(const QString& status);

        void loginStatusChanged(bool success, const QString &token);

        void versionUpdateCheck();
        void versionUpdateAvailable(const QString& updaterDir, const QString& token, const QString& id, const QString& version, const QDateTime& updateTime);

        void provisionProgressChanged(Serial::SerialDevice* device, int status, int current, int error);

        void provisionFailedClose();

    protected slots:
        void viewChanged();
        void viewUpdate();

        void currentDeviceChanged(int);

        void login();
        void logout();

    private:
        void updateLoginStatus(bool loggedIn);

        const QGuiApplication& mApp;

        WebDownloader mDownloader;

        UISection* mSection;

        QQmlApplicationEngine *mEngine;

        ConnectedDevicesModel* mDevicesModel;

        Log::LogHandler* mLogHandler;

        SerialDeviceWorker* mWorker;
        QList<Serial::SerialDevice*> mDevices;

        QTimer* mRescheduleTimer;
        QSet<QString> mRescheduledDevices;

        int mCurrentIndex;
    };

    class UISection : public QObject {
        Q_OBJECT
    public:
        virtual ~UISection() {

        }

        virtual void beforeDeviceChanged();
        virtual void update() = 0;

    protected:
        UISection(MainUI* ui);

        void startUpdate();
        void endUpdate();

        MainUI* ui() {
            return mUI;
        }

        QObject* rootObject() {
            return mUI->engine()->rootObjects().first();
        }

        QList<Serial::SerialDevice*> &devices() {
            return mUI->devices();
        }

        Serial::SerialDevice* currentDevice();

    private:
        MainUI* mUI;
    };
}

#endif // UI_H
