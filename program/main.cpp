#include <QtGlobal>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>

#include "runguard.h"
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
    sLogObject.addLog(type, context, msg);
}

void updateConfigs() {
    Serial::SerialDeviceConfig::updateConfig();
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(common);

    RunGuard runGuard("UPST_main_87f5a82db5098c594efe3b3cefe67cd1b38dcd6c");
    if(!runGuard.tryToRun()) {
        return 0;
    }

    qInstallMessageHandler(&logMessageHandler);

#ifdef Q_OS_WIN
    Utils::WinUtils::enableIntelHack();
#endif

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationDomain("ultimobile.net");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/res/images/icon.svg"));

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

