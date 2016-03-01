#include <QByteArray>
#include <QObject>
#include <QString>

#include "devicefilterevent.h"

DeviceFilterEvent::DeviceFilterEvent()
{

}

#ifdef NATIVEFILTER_MISSING
bool DeviceFilterEvent::nativeEventFilter(const QByteArray &, void*, long*) {
    return false;
}
#endif

void DeviceFilterEvent::handleDeviceAdded() {
    emit devicesChanged();
}

void DeviceFilterEvent::handleDeviceRemoved(const QString& port) {
    emit deviceRemove(port);
}

void DeviceFilterEvent::refresh() {
    handleDeviceAdded();
}
