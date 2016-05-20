#include <QDebug>
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
    case AvailableRole:
        return device->isAvailable();
    case ChildrenSizeRole:
        return device->deviceChildren().size();
    case FlagManualRebootRole:
        return device->flagManualReboot();
    case FlagMultiPortRole:
        return device->flagMultiPort();
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
    roles[NameRole] = "deviceName";
    roles[PortRole] = "devicePort";
    roles[IconRole] = "deviceIcon";
    roles[AvailableRole] = "deviceAvailable";
    roles[ChildrenSizeRole] = "deviceChildrenSize";
    roles[FlagManualRebootRole] = "deviceFlagManualReboot";
    roles[FlagMultiPortRole] = "deviceFlagMultiPort";
    roles[ProgressMaxRole] = "provisionProgressMax";
    roles[ProgressMinRole] = "provisionProgressMin";
    roles[ProgressCurrentRole] = "provisionProgressCurrent";
    roles[ProgressStatusRole] = "provisionProgressStatus";
    roles[ProgressErrorRole] = "provisionProgressError";
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
    case Serial::SerialProvisionErrorRemoved:
        return "The device was removed prematurely.";
    default:
        return "";
    }
}

void UI::ConnectedDevicesModel::deviceChanged(Serial::SerialDevice* device, bool added) {
    if(added) {
        if(mDevices.contains(device)) {
            qWarning()<<"Trying to add a device that's already in the list!!!";
            return;
        }

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
        if(!mDevices.contains(device)) {
            qWarning()<<"Trying to remove a device that's NOT in the list!!!";
            return;
        }

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
    // This may be a provisioned device, which would show up as a new one.
    if(!mDeviceProgress.contains(device)) {
        for(int i = 0; i < mDeviceProgress.size(); i++) {
            Serial::SerialDevice* d = mDeviceProgress.keys().at(i);
            if(device->isSameDevice(device) && d->isProvisioning()) {
                mDeviceProgress.insert(device, mDeviceProgress.value(d));
                mDeviceProgress.remove(d);
                break;
            }
         }
    }

    bool found = false;
    for(int i = 0; i < mDevices.size(); i++) {
        if(mDevices[i]->isSameDevice(device)) {
            found = true;
            mDevices.replace(i, device);

            // The device may have a new parent, so we need to find the child and update it to the new parent.
            if(!mDeviceProgress.contains(device)) {
                foreach(Serial::SerialDevice* child, device->deviceChildren()) {
                    if(mDeviceProgress.contains(child)) {
                        mDeviceProgress.insert(device, mDeviceProgress.value(child));
                        mDeviceProgress.remove(child);
                        break;
                    }
                }
            }

            emit dataChanged(index(i), index(i));
            break;
        }
    }

    // We couldn't find it in the list, add it as a new device
    if(!found) {
        deviceChanged(device, true);
    }
}

void UI::ConnectedDevicesModel::setProgress(Serial::SerialDevice* device, Serial::SerialProvisionStatus status, int current, Serial::SerialProvisionError error) {
    if(mDeviceProgress.find(device) == mDeviceProgress.end()) {
        return;
    }

    qDebug()<<"setProgress"<<status<<current<<error;

    DeviceModelProgress* progress = mDeviceProgress.find(device).value();
    if(progress == nullptr) {
        return;
    }

    progress->status = status;
    progress->error = error;
    progress->current = current;

    deviceUpdate(device);

    qDebug()<<"setProgress done";
}
