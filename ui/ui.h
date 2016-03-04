#ifndef UI_H
#define UI_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "connecteddevicesmodel.h"
#include "serialdeviceworker.h"
#include "serial/serialdevice.h"

namespace UI {
    class UISection;

    class MainUI : public QObject
    {
        Q_OBJECT
    public:
        MainUI(const QGuiApplication& app);
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

    signals:
        void deviceChanged(Serial::SerialDevice* device, bool added);
        void deviceUpdate(Serial::SerialDevice* device);

    public slots:
        void devicesChanged();

        void deviceAdd(Serial::SerialDevice* device);
        void deviceRemove(const QString& port);

        void setStatus(const QString& status);

    protected slots:
        void viewChanged();
        void viewUpdate();

        void currentDeviceChanged(int);

    private:
        const QGuiApplication& mApp;

        UISection* mSection;

        QQmlApplicationEngine *mEngine;

        ConnectedDevicesModel* mDevicesModel;

        SerialDeviceWorker* mWorker;
        QList<Serial::SerialDevice*> mDevices;
    };

    class UISection : public QObject {
        Q_OBJECT
    public:
        virtual ~UISection() {

        }

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
