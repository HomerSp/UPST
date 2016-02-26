#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QAbstractNativeEventFilter>
#include <QQmlContext>
#include <QScreen>
#include <QFile>

#include "ui/ui.h"
#include "devicefilterevent.h"

void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // Skip debug messages when not in testing mode
#ifndef TESTING_MODE
    if(type == QtDebugMsg) {
        return;
    }
#endif
    QString data = "";
    switch(type) {
    case QtDebugMsg:
        data += "[Debug]";
        break;
    case QtWarningMsg:
        data += "[Warning]";
        break;
    default:
        data += "[Error]";
        break;
    }

#ifdef QT_DEBUG
    data += " " + QString(context.file) + "." + QString::number(context.line) + ": ";
#endif
    data += " " + msg + "\n";

    QFile file("log.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
    QTextStream stream(&file);
    stream << data;
    stream.flush();
    file.close();

    QTextStream(stdout) << data;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(&logMessageHandler);

    QGuiApplication app(argc, argv);

    UI::MainUI mainUI(app);

    DeviceFilterEvent deviceFilter;
    QObject::connect(&deviceFilter, &DeviceFilterEvent::devicesChanged, &mainUI, &UI::MainUI::devicesChanged);
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceRemove, &mainUI, &UI::MainUI::deviceRemove);

    app.installNativeEventFilter(&deviceFilter);

    deviceFilter.refresh();

    return app.exec();
}

