#ifndef MAIN
#define MAIN

#include <QQmlApplicationEngine>

#include "serial/serialdevice.h"

class Main : public QObject {
    Q_OBJECT
public:
    Main(QQmlApplicationEngine* engine);
    ~Main();

signals:
    void deviceChanged(const Serial::SerialDevice& device, bool added);

public slots:
    void deviceAdd(Serial::SerialDevice* device);
    void deviceRemove(const QString& port);

    void viewChanged();
    void viewUpdate();

    void provision();

private:
    QQmlApplicationEngine *mEngine;

    QList<Serial::SerialDevice*> mDevices;
};

#endif // MAIN

