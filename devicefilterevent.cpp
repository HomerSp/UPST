#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSerialPortInfo>
#include <QThread>

#include "devicefilterevent.h"

DeviceFilterEvent::DeviceFilterEvent()
{

}

#ifdef NATIVEFILTER_MISSING
bool DeviceFilterEvent::nativeEventFilter(const QByteArray &eventType, void* message, long*) {
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
