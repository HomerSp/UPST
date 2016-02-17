#ifndef UI_H
#define UI_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "connecteddevicesmodel.h"
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

    signals:
        void deviceChanged(Serial::SerialDevice* device, bool added);

    public slots:
        void deviceAdd(Serial::SerialDevice* device);
        void deviceRemove(const QString& port);

    protected slots:
        void viewChanged();
        void viewUpdate();

        void currentDeviceChanged(int);

    private:
        const QGuiApplication& mApp;

        UISection* mSection;

        QQmlApplicationEngine *mEngine;

        ConnectedDevicesModel* mDevicesModel;

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
