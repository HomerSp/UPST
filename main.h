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

    void addDevice(Serial::SerialDevice* device);

public slots:
    void deviceChanged(Serial::SerialDevice* device, bool added);

private:
    QList<Serial::SerialDevice*> mDevices;
};


class Main : public QObject {
    Q_OBJECT
public:
    Main(QQmlApplicationEngine* engine);
    ~Main();

signals:
    void deviceChanged(Serial::SerialDevice* device, bool added);

public slots:
    void deviceAdd(Serial::SerialDevice* device);
    void deviceRemove(const QString& port);

    void viewChanged();
    void viewUpdate();

    void currentDeviceChanged(int);

    void provision();

private:
    QQmlApplicationEngine *mEngine;

    QList<Serial::SerialDevice*> mDevices;
};

#endif // MAIN

