#ifndef SERIALDEVICECONFIG_H
#define SERIALDEVICECONFIG_H

#include <QCoreApplication>
#include <QJsonDocument>
#include "serialdevice.h"

namespace Serial {
    class SerialDeviceConfig
    {
    public:
        SerialDeviceConfig(const QString& data);
        ~SerialDeviceConfig();

        bool isValid() const {
            return mIsValid;
        }

        QList<SerialDevice*> getDeviceGuides();

        bool shouldReschedule(SerialDevice* device);
        bool updateDevice(SerialDevice* device);

        static void updateConfig();

    private:
        bool update(const QString& data);

        bool mIsValid;
        QJsonDocument mDevices;
    };
}

#endif // SERIALDEVICECONFIG_H
