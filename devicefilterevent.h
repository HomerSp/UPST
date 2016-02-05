#ifndef DEVICEFILTEREVENT_H
#define DEVICEFILTEREVENT_H

#include <QObject>
#include <QAbstractNativeEventFilter>

#include "serial/serialdevice.h"

class DeviceFilterEvent: public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    DeviceFilterEvent();

    virtual bool nativeEventFilter(const QByteArray &eventType, void* message, long*) Q_DECL_OVERRIDE;

    void refresh();

signals:
    void deviceAdd(Serial::SerialDevice* device);
    void deviceRemove(const QString& port);

protected:
    void handleDeviceAdded();
    void handleDeviceRemoved(const QString& port);

    bool checkAndroidDevice(const QString& port, const QString& vid, const QString& pid);

private:
    bool getVidPid(const QString& port, QString& vid, QString& pid);
};

#endif // DEVICEFILTEREVENT_H
