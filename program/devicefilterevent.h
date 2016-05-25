#ifndef DEVICEFILTEREVENT_H
#define DEVICEFILTEREVENT_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QSet>

#include "serial/serialdevice.h"

class DeviceFilterEvent: public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    DeviceFilterEvent();

    virtual bool nativeEventFilter(const QByteArray &eventType, void* message, long*) Q_DECL_OVERRIDE;

public slots:
    void enable();
    void disable();

    void refresh();

signals:
    void deviceAdd(const QString& port);
    void deviceRemove(const QString& port);

protected:
    bool enabled() {
        return mEnabled;
    }

    virtual QSet<QString> getDevices() = 0;

    virtual void process();

private:
    bool mEnabled;

    QSet<QString> mConnectedDevices;

};

#endif // DEVICEFILTEREVENT_H
