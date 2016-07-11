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
    refresh();
}

bool DeviceFilterEvent::nativeEventFilter(const QByteArray &, void*, long*) {
    return false;
}

void DeviceFilterEvent::refresh() {
    mConnectedDevices.clear();
    process();
}

void DeviceFilterEvent::process() {
    if(!enabled()) {
        return;
    }

    QMap<QString, DevicePortInfo*> devices = getDevices();
    if(mConnectedDevices.size() > 0) {
        for(int i = 0; i < mConnectedDevices.size(); i++) {
            QString port = mConnectedDevices.keys().at(i);

            bool found = false;
            foreach(QString d, devices.keys()) {
                if(port == d) {
                    found = true;
                    break;
                }
            }

            if(!found) {
                mConnectedDevices.remove(port);
                emit deviceRemove(port);
                i--;
            }
        }
    }

    foreach(QString d, devices.keys()) {
        bool found = false;
        foreach(QString cd, mConnectedDevices.keys()) {
            if(d == cd) {
                found = true;
                break;
            }
        }

        if(!found) {
            emit deviceAdd(d);
            mConnectedDevices.insert(d, devices.value(d));
        }
    }
}
