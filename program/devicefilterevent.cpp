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
    mConnectedDevices.clear();
}

bool DeviceFilterEvent::nativeEventFilter(const QByteArray &, void*, long*) {
    return false;
}

void DeviceFilterEvent::refresh() {
    process();
}

void DeviceFilterEvent::process() {
    if(!enabled()) {
        return;
    }

    QSet<QString> devices = getDevices();
    if(mConnectedDevices.size() > 0) {
        for(int i = 0; i < mConnectedDevices.size(); i++) {
            QString port = *(mConnectedDevices.begin() + i);

            bool found = false;
            foreach(QString d, devices) {
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

    foreach(QString d, devices) {
        bool found = false;
        for(int i = 0; i < mConnectedDevices.size(); i++) {
            QString port = *(mConnectedDevices.begin() + i);
            if(port == d) {
                found = true;
                break;
            }
        }

        if(!found) {
            emit deviceAdd(d);
            mConnectedDevices.insert(d);
        }
    }
}
