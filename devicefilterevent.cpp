#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSerialPortInfo>

#include "main.h"
#include "devicefilterevent.h"

DeviceFilterEvent::DeviceFilterEvent()
{

}

void DeviceFilterEvent::handleDeviceChanged(const QString &port, bool added) {
    qDebug()<<"handleDeviceChanged availablePorts"<<QSerialPortInfo::availablePorts().size();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if(info.portName() == port) {
            QString port, vid, pid;
            port = info.portName();
            vid.sprintf("%04X", info.vendorIdentifier());
            pid.sprintf("%04X", info.productIdentifier());

            qDebug()<<"Device"<<port<<vid<<pid<<((added)?"added":"removed");

            if(added) {
                Serial::SerialDevice* device = new Serial::SerialDevice(info, true);
                if(device->isValid()) {
                    qDebug()<<"Device is valid"<<device->port();
                    emit deviceAdd(device);
                } else {
                    qDebug()<<"Device is not valid"<<device->port();
                    delete device;
                }
            } else {
                emit deviceRemove(port);
            }
        }
    }
}

void DeviceFilterEvent::refresh() {
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        handleDeviceChanged(info.portName(), true);
    }
}
