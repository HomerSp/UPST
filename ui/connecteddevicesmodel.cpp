#include "connecteddevicesmodel.h"

UI::ConnectedDevicesModel::ConnectedDevicesModel(QObject* parent)
    : QAbstractListModel(parent) {

}

UI::ConnectedDevicesModel::~ConnectedDevicesModel() {
    while(!mDeviceProgress.empty()) {
        delete mDeviceProgress.begin().value();
        mDeviceProgress.erase(mDeviceProgress.begin());
    }
}

QVariant UI::ConnectedDevicesModel::data(const QModelIndex& index, int role) const {
    Serial::SerialDevice* device = mDevices.at(index.row());
    DeviceModelProgress* progress = mDeviceProgress.find(device).value();

    switch(role) {
    case NameRole:
        return (device->name().length() > 0)?device->name():"Unknown";
    case PortRole:
        return device->portStr();
    case IconRole:
        return getDeviceIcon(device);
    case ProgressMaxRole:
        return progress->max;
    case ProgressMinRole:
        return progress->min;
    case ProgressCurrentRole:
        return progress->current;
    case ProgressStatusRole:
        return progress->status;
    case ProgressErrorRole:
        return getProgressError(progress->error);
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
    roles[ProgressMaxRole] = "progressMax";
    roles[ProgressMinRole] = "progressMin";
    roles[ProgressCurrentRole] = "progressCurrent";
    roles[ProgressStatusRole] = "progressStatus";
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

QString UI::ConnectedDevicesModel::getProgressError(Serial::SerialProvisionError error) const {
    switch(error) {
    case Serial::SerialProvisionErrorData:
        return "Failed to parse provision data.";
    case Serial::SerialProvisionErrorDownload:
        return "Failed to download provision data.";
    case Serial::SerialProvisionErrorNv:
        return "Failed to write one or more items.";
    default:
        return "";
    }
}

void UI::ConnectedDevicesModel::deviceChanged(Serial::SerialDevice* device, bool added) {
    if(added) {
        QAbstractListModel::beginInsertRows(QModelIndex(), mDevices.size(), mDevices.size());
        mDevices.append(device);

        DeviceModelProgress* progress = new DeviceModelProgress();
        progress->status = Serial::SerialProvisionStatusIdle;
        progress->error = Serial::SerialProvisionErrorNone;
        progress->current = 0;
        progress->max = 100;
        progress->min = 0;
        mDeviceProgress.insert(device, progress);

        QAbstractListModel::endInsertRows();
    } else {
        for(int i = 0; i< mDevices.size(); i++) {
            if(mDevices[i] == device) {
                QAbstractListModel::beginRemoveRows(QModelIndex(), i, i);
                mDevices.removeAt(i);

                if(mDeviceProgress.contains(device)) {
                    delete mDeviceProgress.find(device).value();
                    mDeviceProgress.remove(device);
                }
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

void UI::ConnectedDevicesModel::setProgress(Serial::SerialDevice* device, Serial::SerialProvisionStatus status, int current, Serial::SerialProvisionError error) {
    DeviceModelProgress* progress = mDeviceProgress.find(device).value();
    progress->status = status;
    progress->error = error;
    progress->current = current;

    deviceUpdate(device);
}
