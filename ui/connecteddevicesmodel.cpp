#include "connecteddevicesmodel.h"

UI::ConnectedDevicesModel::ConnectedDevicesModel(QObject* parent)
    : QAbstractListModel(parent) {

}

QVariant UI::ConnectedDevicesModel::data(const QModelIndex& index, int role) const {
    Serial::SerialDevice* device = mDevices.at(index.row());
    switch(role) {
    case NameRole:
        return device->name();
    case PortRole:
        return device->portStr();
    case IconRole:
        return getDeviceIcon(device);
    }

    return "";
}
int UI::ConnectedDevicesModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);

    return mDevices.size();
}

QHash<int, QByteArray> UI::ConnectedDevicesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PortRole] = "port";
    roles[IconRole] = "icon";
    return roles;
}

QString UI::ConnectedDevicesModel::getDeviceIcon(Serial::SerialDevice* device) const {
    QString type = "unknown";

    switch(device->type()) {
    case Serial::SerialDeviceTypeSmartphone:
        type = "smartphone";
        break;
    case Serial::SerialDeviceTypeFeaturePhone:
        type = "feature_phone";
        break;
    case Serial::SerialDeviceTypeTablet:
        type = "tablet";
        break;
    case Serial::SerialDeviceTypeMifi:
        type = "mifi";
        break;
    default:
        break;
    }

    return "qrc:/res/images/icons/" + type + ".svg";
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

void UI::ConnectedDevicesModel::deviceUpdate(Serial::SerialDevice* device) {
    for(int i = 0; i < mDevices.size(); i++) {
        if(*(mDevices[i]) == *(device)) {
            mDevices.replace(i, device);
            emit dataChanged(index(i), index(i));
            break;
        }
    }
}
