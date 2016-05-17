#ifndef CONNECTEDDEVICESMODEL_H
#define CONNECTEDDEVICESMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "serial/serialdevice.h"

namespace UI {
    struct DeviceModelProgress {
        Serial::SerialProvisionStatus status;
        Serial::SerialProvisionError error;
        int min;
        int max;
        int current;
    };

    class ConnectedDevicesModel : public QAbstractListModel {
        Q_OBJECT
    public:
        enum DevicesRoles {
            NameRole = Qt::UserRole + 1,
            PortRole,
            IconRole,
            AvailableRole,
            ManualRebootRole,
            ProgressMaxRole,
            ProgressMinRole,
            ProgressCurrentRole,
            ProgressStatusRole,
            ProgressErrorRole,
        };

        ConnectedDevicesModel(QObject* parent = 0);
        ~ConnectedDevicesModel();

        virtual QVariant data(const QModelIndex& parent, int role) const;
        virtual QHash<int, QByteArray> roleNames() const;
        virtual int rowCount(const QModelIndex &parent) const;

        void addDevice(Serial::SerialDevice* device);

    public slots:
        void deviceChanged(Serial::SerialDevice* device, bool added);
        void deviceUpdate(Serial::SerialDevice* device);

        void setProgress(Serial::SerialDevice* device, int status, int current, int error);

    private:
        QString getDeviceIcon(Serial::SerialDevice* device) const;
        QString getProgressError(Serial::SerialProvisionError error) const;

        QList<Serial::SerialDevice*> mDevices;
        QMap<Serial::SerialDevice*, DeviceModelProgress*> mDeviceProgress;
    };
}

#endif // CONNECTEDDEVICESMODEL_H
