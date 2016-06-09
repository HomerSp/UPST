#ifndef UI_H
#define UI_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTemporaryDir>
#include <QTimer>
#include <QSet>

#include "connecteddevicesmodel.h"
#include "loghandler.h"
#include "worker/uiworker.h"
#include "worker/serialdeviceworker.h"

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
        MainUI(const QGuiApplication& app, Log::LogHandler* logHandler, bool updateFailed);
        ~MainUI();

        QQmlApplicationEngine* engine() {
            return mEngine;
        }

        QList<Serial::SerialDevice*> &devices() {
            return mDevices;
        }

        UI::Worker::UIWorker* worker() {
            return mWorker;
        }

        UI::Worker::SerialDeviceWorker* deviceWorker() {
            return mDeviceWorker;
        }

        ConnectedDevicesModel* devicesModel() {
            return mDevicesModel;
        }

        int currentIndex() {
            return mCurrentIndex;
        }

    signals:
        void devicesRefresh();
        void deviceChanged(Serial::SerialDevice* device, bool added);
        void deviceUpdate(Serial::SerialDevice* device);

        void loggedIn();
        void loggedOut();

    public slots:
        void devicesListChanged(bool success, Serial::SerialDeviceConfig* config);

        void loginStatusChanged(bool success);

        void versionUpdateCheck();
        void versionUpdateAvailable(const QString& updaterDir, const QString& id, const QString& version, const QDateTime& updateTime);

        void deviceAddChecked(Serial::SerialDevice* device);
        void deviceAddReschedule(QString port);
        void deviceClose(Serial::SerialDevice* device);
        void deviceRescheduleTimeout();

        void provisionProgressChanged(Serial::SerialDevice* device, int status, int current, int error);

        void deviceAdd(const QString& port);
        void deviceRemove(const QString& port);

        void setStatus(const QString& status);

        void doRefresh();

        void provisionFailedClose();
        void updateFailedContinue();

    protected slots:
        void viewChanged();
        void viewUpdate();

        void currentDeviceChanged(int);

        void login();
        void logout();

        void importDevices(const QUrl& url);

    private:
        void updateLoginStatus(bool loggedIn);

        const QGuiApplication& mApp;

        WebDownloader mDownloader;

        UISection* mSection;

        QQmlApplicationEngine *mEngine;

        ConnectedDevicesModel* mDevicesModel;

        Log::LogHandler* mLogHandler;

        UI::Worker::UIWorker* mWorker;
        UI::Worker::SerialDeviceWorker* mDeviceWorker;

        QList<Serial::SerialDevice*> mDevices;
        QMap<QString, int> mDeviceRechecks;

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
