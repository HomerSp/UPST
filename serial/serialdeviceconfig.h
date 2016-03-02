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
        enum NvType {
            NvTypeNone,
            NvType8Bit,
            NvType16Bit,
            NvType32Bit,
            NvType64Bit,
            NvTypeString,
        };

        bool checkNvItem(SerialDevice* device, uint16_t id, NvType type, const QVariant& checkData, QVariant& outData);

        QVariant getValue(const QString& type, const QString& checkData, NvType& outType);

        QJsonDocument mDevices;
    };
}

#endif // SERIALDEVICECONFIG_H
