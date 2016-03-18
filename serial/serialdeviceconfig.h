#ifndef SERIALDEVICECONFIG_H
#define SERIALDEVICECONFIG_H

#include <QCoreApplication>
#include <QJsonDocument>
#include "serialdevice.h"

namespace Serial {
    class SerialDeviceConfig
    {
    public:
        SerialDeviceConfig();
        ~SerialDeviceConfig();

        bool updateDevice(SerialDevice* device);

        static void updateConfig();

    private:
        QJsonDocument mDevices;
    };
}

#endif // SERIALDEVICECONFIG_H
