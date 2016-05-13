#include <QtGlobal>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>

#include "utils/fileutils.h"
#ifdef Q_OS_WIN
#include "utils/winutils.h"
#endif
#include "ui/ui.h"
#include "devicefilterevent.h"

#include "serial/serialdeviceconfig.h"

static UI::LogObject sLogObject;

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
    data += " " + QString(context.file) + "." + QString::number(context.line) + ":";
#endif
    data += " " + msg;

    if(data.length() > 0) {
        if(data.at(data.length() - 1) != '\n') {
            data += "\n";
        }
    }

    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/upst.log");
    if(file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << data;
        stream.flush();
        file.close();
    }

    QTextStream(stdout) << data;

    sLogObject += data;
    if(LOG_LIMIT > 0 && sLogObject.length() > LOG_LIMIT) {
        sLogObject.remove(0, sLogObject.length() - LOG_LIMIT);
    }
}

void updateConfigs() {
    Serial::SerialDeviceConfig::updateConfig();
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(common);

    qInstallMessageHandler(&logMessageHandler);

#ifdef Q_OS_WIN
    Utils::WinUtils::enableIntelHack();
#endif

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationDomain("ultimobile.net");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QGuiApplication app(argc, argv);

    if(app.arguments().size() == 3 && app.arguments().at(1) == "update") {
        QString updatePath = app.arguments().at(2);

        qDebug()<<"Deleting temporary files from"<<updatePath;

        QList<QString> updaterFiles, updaterDirs;
        Utils::FileUtils::listFiles(updaterFiles, updatePath);
        Utils::FileUtils::listDirs(updaterDirs, updatePath);

        int retries = 0;
        bool retry = false;
        do {
            if(retries >= 5) {
                qWarning()<<"Failed to delete temporary files...";
                break;
            }

            foreach(QString file, updaterFiles) {
                if(!QFile::remove(updatePath + "/" + file)) {
                    break;
                }
            }

            if(retry) {
                retries++;
                QThread::sleep(5);
            }
        } while(retry);

        foreach(QString dir, updaterDirs) {
            QDir(updatePath).rmdir(dir);
        }

        QDir().rmdir(updatePath);
    }

    qInfo()<<"Starting UPST"<<PROG_VERSION<<"at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

    updateConfigs();

    UI::MainUI mainUI(app, &sLogObject);

    DeviceFilterEvent deviceFilter;
    QObject::connect(&deviceFilter, &DeviceFilterEvent::devicesChanged, &mainUI, &UI::MainUI::devicesChanged);
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceRemove, &mainUI, &UI::MainUI::deviceRemove);
    QObject::connect(&mainUI, &UI::MainUI::loggedIn, &deviceFilter, &DeviceFilterEvent::enable);
    QObject::connect(&mainUI, &UI::MainUI::loggedOut, &deviceFilter, &DeviceFilterEvent::disable);

    app.installNativeEventFilter(&deviceFilter);

    return app.exec();
}

