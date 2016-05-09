#include <QtGlobal>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

#include "ui/ui.h"
#include "devicefilterevent.h"

#include "serial/serialdeviceconfig.h"

static QString sLogData = "";

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
    case QtInfoMsg:
        data += "[Info]";
        break;
    case QtWarningMsg:
        data += "[Warning]";
        break;
    case QtCriticalMsg:
        data += "[Critical]";
        break;
    case QtFatalMsg:
        data += "[Fatal]";
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

    sLogData += data + "\n";
    if(LOG_LIMIT > 0 && sLogData.length() > LOG_LIMIT) {
        sLogData.remove(0, sLogData.length() - LOG_LIMIT);
    }
}

void updateConfigs() {
    Serial::SerialDeviceConfig::updateConfig();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(&logMessageHandler);

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationDomain("ultimobile.net");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QGuiApplication app(argc, argv);

    qInfo()<<"Starting UPST"<<PROG_VERSION<<"at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

    updateConfigs();

    UI::MainUI mainUI(app, sLogData);

    DeviceFilterEvent deviceFilter;
    QObject::connect(&deviceFilter, &DeviceFilterEvent::devicesChanged, &mainUI, &UI::MainUI::devicesChanged);
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceRemove, &mainUI, &UI::MainUI::deviceRemove);
    QObject::connect(&mainUI, &UI::MainUI::loggedIn, &deviceFilter, &DeviceFilterEvent::enable);
    QObject::connect(&mainUI, &UI::MainUI::loggedOut, &deviceFilter, &DeviceFilterEvent::disable);

    app.installNativeEventFilter(&deviceFilter);

    return app.exec();
}

