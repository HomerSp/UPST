#ifndef DEVICEGUIDELISTMODEL_H
#define DEVICEGUIDELISTMODEL_H

#include <QAbstractListModel>
#include <QMap>

#include "serial/serialdevice.h"
#include "serial/serialdeviceconfig.h"

namespace UI {
namespace Guide {
    struct DeviceGuideListItem {
        QString Header;
        Serial::SerialDevice* Device;
    };

    class DeviceGuideListModel : public QAbstractListModel {
        Q_OBJECT
    public:
        enum DevicesRoles {
            HeaderRole = Qt::UserRole + 1,
            MakeRole,
            ModelRole,
            CodenameRole,
            TypeRole,
            IconRole,
        };

        DeviceGuideListModel(QObject* parent = 0);
        ~DeviceGuideListModel();

        virtual QVariant data(const QModelIndex& parent, int role) const;
        virtual QHash<int, QByteArray> roleNames() const;
        virtual int rowCount(const QModelIndex &parent) const;

        Q_INVOKABLE int startIndex();

        Q_INVOKABLE QString deviceData(int index);

        void setDeviceConfig(Serial::SerialDeviceConfig* config);

    private:
        QString getDeviceIcon(Serial::SerialDevice* device) const;

        QList<DeviceGuideListItem*> mItems;
    };
}
}

#endif // DEVICEGUIDELISTMODEL_H
