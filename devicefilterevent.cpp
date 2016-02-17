#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSerialPortInfo>

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
    qDebug()<<"handleDeviceAdded availablePorts"<<QSerialPortInfo::availablePorts().size();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString port, vid, pid;
        port = info.portName();
        vid.sprintf("%04X", info.vendorIdentifier());
        pid.sprintf("%04X", info.productIdentifier());

        qDebug()<<"Device"<<port<<vid<<pid<<"added";

        Serial::SerialDevice* device = new Serial::SerialDevice(info, true);
        if(device->isValid()) {
            qDebug()<<"Device is valid"<<device->port();
            emit deviceAdd(device);
        } else {
            qWarning()<<"Device is not valid"<<device->port();
            delete device;
        }
    }
}

void DeviceFilterEvent::handleDeviceRemoved(const QString& port) {
    emit deviceRemove(port);
}

void DeviceFilterEvent::refresh() {
    handleDeviceAdded();
}
