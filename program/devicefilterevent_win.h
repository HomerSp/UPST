#ifndef DEVICEFILTEREVENT_WIN_H
#define DEVICEFILTEREVENT_WIN_H

#include <QSet>

#include "devicefilterevent.h"

class DeviceFilterEventWin: public DeviceFilterEvent{
    Q_OBJECT
public:
    DeviceFilterEventWin();

    virtual bool nativeEventFilter(const QByteArray &eventType, void* message, long*) Q_DECL_OVERRIDE;

protected:
    virtual QSet<QString> getDevices();
};

#endif // DEVICEFILTEREVENT_WIN_H
