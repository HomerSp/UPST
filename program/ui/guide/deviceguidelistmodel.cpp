#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QJsonArray>

#include "web/webutils.h"
#include "deviceguidelistmodel.h"

using namespace UI::Guide;

DeviceGuideListModel::DeviceGuideListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

DeviceGuideListModel::~DeviceGuideListModel() {
    foreach(DeviceGuideListItem* item, mItems) {
        if(item->Device != nullptr) {
            delete item->Device;
        }
        item->Device = nullptr;

        delete item;
    }
}

QVariant DeviceGuideListModel::data(const QModelIndex& index, int role) const {
    DeviceGuideListItem* item = mItems.at(index.row());
    if(item == nullptr) {
        return "";
    }

    if(item->Header.size() > 0) {
        switch(role) {
        case HeaderRole:
            return true;
        case MakeRole:
            return item->Header;
        default:
            break;
        }

        return "";
    }

    if(item->Device == nullptr) {
        return "";
    }

    switch(role) {
    case HeaderRole:
        return false;
    case MakeRole:
        return item->Device->make();
    case ModelRole:
        return item->Device->model();
    case CodenameRole:
        return item->Device->codename();
    case IconRole:
        return getDeviceIcon(item->Device);
    }

    return "";
}

int DeviceGuideListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);

    return mItems.size();
}

QHash<int, QByteArray> DeviceGuideListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[HeaderRole] = "deviceHeader";
    roles[MakeRole] = "deviceMake";
    roles[ModelRole] = "deviceModel";
    roles[CodenameRole] = "deviceCodename";
    roles[IconRole] = "deviceIcon";
    return roles;
}

QString DeviceGuideListModel::getDeviceIcon(Serial::SerialDevice* device) const {
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

void DeviceGuideListModel::setDeviceConfig(Serial::SerialDeviceConfig* config) {
    if(config != nullptr) {
        QList<Serial::SerialDevice*> devices = config->getDeviceGuides();

        QAbstractListModel::beginInsertRows(QModelIndex(), 0, devices.size());

        QStringList makes;
        QStringList models;
        foreach(Serial::SerialDevice* device, devices) {
            if(!makes.contains(device->make())) {
                makes.append(device->make());
            }
            if(!models.contains(device->model())) {
                models.append(device->model());
            }
        }

        makes.sort();
        models.sort();

        foreach(QString make, makes) {
            DeviceGuideListItem* headerItem = new DeviceGuideListItem;
            headerItem->Header = make;
            headerItem->Device = nullptr;
            mItems.append(headerItem);

            foreach(QString model, models) {
                foreach(Serial::SerialDevice* device, devices) {
                    if(device->make() == make && device->model() == model) {
                        DeviceGuideListItem* deviceItem = new DeviceGuideListItem;
                        deviceItem->Header = "";
                        deviceItem->Device = device;
                        mItems.append(deviceItem);
                    }
                }
            }
        }

        QAbstractListModel::endInsertRows();
    }
}

QString DeviceGuideListModel::deviceData(int index) {
    const QJsonObject &obj = mItems.at(index)->Device->guide();

    QString data = "<!DOCTYPE HTML>\
            <html>\
                <head>\
                    <meta charset=\"UTF-8\">\
                        <title>Document</title>\
                        <style type=\"text/css\">\
                        body { color: #262626; margin: 0; padding: 0; }\
                        h1 { color: white; margin: 0; padding: 0; }\
                        h3 { color: white; margin: 0; padding: 0; }\
                        h2 { color: #288CF2; margin-bottom: 12px; }\
                        p { color: #262626; font-size: 0.9em; margin: 0; padding: 0; }\
                        #body { padding-bottom: 150px; }\
                        #content { padding: 20px; }\
                        #content tr { height: 400px; }\
                        #content td { vertical-align: top; }\
                        .content-left { vertical-align: top; height: 400px; } \
                        .content-right { width: 450px; text-align: right; } \
                    </style>\
                    </head>\
                    <body>\
                        <div id=\"body\">\
                            <div id=\"content\">\
                                <table width=\"100%\" cellspacing=\"20\">\
                                    <tr>\
                                        <td class=\"content-left\">";

    QJsonArray steps = obj["data"].toArray();
    for(int i = 0; i < steps.size(); i++) {
        QJsonObject step = steps.at(i).toObject();
        data += "<h2>" + QString::number(i + 1) + " - " + step["header"].toString("") + "</h2>";

        data += "<p>";

        QJsonArray lines = step["lines"].toArray();
        for(int y = 0; y < lines.size(); y++) {
            data += QString::number(y + 1) + ". " + lines.at(y).toString("");
            if(y < lines.size() - 1) {
                data += "<br/>";
            }
        }
        data += "</p>";
        if(i < steps.size() - 1) {
            data += "<br/>";
        }
    }

    data += "</td>";

    if(obj.contains("image") && obj["image"].toString().length() > 0) {
        data += "<td width=\"450\" class=\"content-right\"><img src=\"" + obj["image"].toString() + "\" align=\"right\"/></td>";
    }
    data += "</tr></table></div></div></body>";

    return data;
}

int DeviceGuideListModel::startIndex() {
    for(int i = 0; i < mItems.size(); i++) {
        if(mItems.at(i)->Header.size() == 0) {
            return i;
        }
    }

    return 0;
}
