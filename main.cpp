#include <QtGlobal>
#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

#include "ui/ui.h"
#include "devicefilterevent.h"
#include "serial/serialdeviceconfig.h"

void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);
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

    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/log.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
    QTextStream stream(&file);
    stream << data;
    stream.flush();
    file.close();

    QTextStream(stdout) << data;
}

void updateConfigs() {
    Serial::SerialDeviceConfig::updateConfig();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(&logMessageHandler);

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QGuiApplication app(argc, argv);

    updateConfigs();

    UI::MainUI mainUI(app);

    DeviceFilterEvent deviceFilter;
    QObject::connect(&deviceFilter, &DeviceFilterEvent::devicesChanged, &mainUI, &UI::MainUI::devicesChanged);
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceRemove, &mainUI, &UI::MainUI::deviceRemove);
    QObject::connect(&mainUI, &UI::MainUI::loggedIn, &deviceFilter, &DeviceFilterEvent::enable);
    QObject::connect(&mainUI, &UI::MainUI::loggedOut, &deviceFilter, &DeviceFilterEvent::disable);

    app.installNativeEventFilter(&deviceFilter);

    return app.exec();
}

