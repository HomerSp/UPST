#ifndef CONNECTEDDEVICESMODEL_H
#define CONNECTEDDEVICESMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "serial/serialdevice.h"

namespace UI {
    class ConnectedDevicesModel : public QAbstractListModel {
        Q_OBJECT
    public:
        enum DevicesRoles {
            NameRole = Qt::UserRole + 1,
            PortRole
        };

        ConnectedDevicesModel(QObject* parent = 0);

        virtual QVariant data(const QModelIndex& parent, int role) const;
        virtual QHash<int, QByteArray> roleNames() const;
        virtual int rowCount(const QModelIndex &parent) const;

        void addDevice(Serial::SerialDevice* device);

    public slots:
        void deviceChanged(Serial::SerialDevice* device, bool added);

    private:
        QList<Serial::SerialDevice*> mDevices;
    };
}

#endif // CONNECTEDDEVICESMODEL_H
