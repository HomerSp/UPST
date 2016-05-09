#include <QByteArray>
#include <QObject>
#include <QString>

#include "devicefilterevent.h"

DeviceFilterEvent::DeviceFilterEvent()
    : mEnabled(false)
{

}

void DeviceFilterEvent::enable() {
    mEnabled = true;

    refresh();
}

void DeviceFilterEvent::disable() {
    mEnabled = false;


}

#ifdef NATIVEFILTER_MISSING
bool DeviceFilterEvent::nativeEventFilter(const QByteArray &, void*, long*) {
    return false;
}
#endif

void DeviceFilterEvent::handleDeviceAdded() {
    if(!mEnabled) {
        return;
    }

    emit devicesChanged();
}

void DeviceFilterEvent::handleDeviceRemoved(const QString& port) {
    if(!mEnabled) {
        return;
    }

    emit deviceRemove(port);
}

void DeviceFilterEvent::refresh() {
    handleDeviceAdded();
}
