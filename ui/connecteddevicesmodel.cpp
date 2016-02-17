#include "connecteddevicesmodel.h"

UI::ConnectedDevicesModel::ConnectedDevicesModel(QObject* parent)
    : QAbstractListModel(parent) {

}

QVariant UI::ConnectedDevicesModel::data(const QModelIndex& index, int role) const {
    Serial::SerialDevice* device = mDevices.at(index.row());
    switch(role) {
    case NameRole:
        return device->description();
    case PortRole:
        return device->port();
    }

    return "";
}
int UI::ConnectedDevicesModel::rowCount(const QModelIndex &parent) const {
    return mDevices.size();
}

QHash<int, QByteArray> UI::ConnectedDevicesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PortRole] = "port";
    return roles;
}

void UI::ConnectedDevicesModel::deviceChanged(Serial::SerialDevice* device, bool added) {
    if(added) {
        QAbstractListModel::beginInsertRows(QModelIndex(), mDevices.size(), mDevices.size());
        mDevices.append(device);
        QAbstractListModel::endInsertRows();
    } else {
        for(int i = 0; i< mDevices.size(); i++) {
            if(mDevices[i] == device) {
                QAbstractListModel::beginRemoveRows(QModelIndex(), i, i);
                mDevices.removeAt(i);
                QAbstractListModel::endRemoveRows();
                break;
            }
        }
    }
}
