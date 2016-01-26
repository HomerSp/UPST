#ifndef MAIN
#define MAIN

#include <QAbstractListModel>
#include <QQmlApplicationEngine>
#include <QPair>

#include "serial/serialdevice.h"

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

    void addDevice(const QString& name, const QString& port);

private:
    QList<QPair<QString, QString>> mDevices;
};


class Main : public QObject {
    Q_OBJECT
public:
    Main(QQmlApplicationEngine* engine, ConnectedDevicesModel* model);
    ~Main();

signals:
    void deviceChanged(const Serial::SerialDevice& device, bool added);

public slots:
    void deviceAdd(Serial::SerialDevice* device);
    void deviceRemove(const QString& port);

    void viewChanged();
    void viewUpdate();

    void currentDeviceChanged(int);

    void provision();

private:
    QQmlApplicationEngine *mEngine;

    ConnectedDevicesModel* mModel;

    QList<Serial::SerialDevice*> mDevices;
};

#endif // MAIN

