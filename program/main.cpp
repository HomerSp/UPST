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

#ifdef Q_OS_WIN
#include "devicefilterevent_win.h"
#else
#include "devicefilterevent.h"
#endif

#include "serial/serialdeviceconfig.h"

static Log::LogHandler *sLogHandler = nullptr;

void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if(sLogHandler != nullptr) {
        sLogHandler->addLog(type, context, msg);
    }
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

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationDomain("ultimobile.net");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    sLogHandler = new Log::LogHandler("upst.log");

    qInstallMessageHandler(&logMessageHandler);

#ifdef Q_OS_WIN
    Utils::WinUtils::enableIntelHack();
#endif

    QGuiApplication *app = new QGuiApplication(argc, argv);
    app->setWindowIcon(QIcon(":/res/images/icon.svg"));

    bool updateFailed = false;
    if(app->arguments().size() == 4 && app->arguments().at(1) == "update") {
        updateFailed = (app->arguments().at(2) != PROG_BUILDTIME);

        QString updatePath = app->arguments().at(3);

        qDebug()<<"Deleting temporary files from"<<updatePath;

        QList<QString> updaterFiles, updaterDirs;
        Utils::FileUtils::listFiles(updaterFiles, updatePath);
        Utils::FileUtils::listDirs(updaterDirs, updatePath);

        // Don't try to remove something we shouldn't.
        if(updaterFiles.contains("UPST.exe") && updaterFiles.contains("Updater.exe")) {
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
        } else {
            qWarning()<<"Trying to remove"<<updatePath<<"which doesn't seem like a valid directory...";
        }
    }

    qInfo()<<"Starting UPST"<<PROG_VERSION<<"at"<<QDateTime::currentDateTime().toString(Qt::ISODate);
    if(updateFailed) {
        qCritical()<<"The update failed, please report this";
    }

    updateConfigs();

    UI::MainUI* mainUI = new UI::MainUI(*app, sLogHandler, updateFailed);

#ifdef Q_OS_WIN
    DeviceFilterEventWin deviceFilter;
#else
    DeviceFilterEvent deviceFilter;
#endif
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceAdd, mainUI, &UI::MainUI::deviceAdd);
    QObject::connect(&deviceFilter, &DeviceFilterEvent::deviceRemove, mainUI, &UI::MainUI::deviceRemove);
    QObject::connect(mainUI, &UI::MainUI::loggedIn, &deviceFilter, &DeviceFilterEvent::enable);
    QObject::connect(mainUI, &UI::MainUI::loggedOut, &deviceFilter, &DeviceFilterEvent::disable);
    QObject::connect(mainUI, &UI::MainUI::devicesRefresh, &deviceFilter, &DeviceFilterEvent::refresh);

    app->installNativeEventFilter(&deviceFilter);

    int ret = app->exec();

    delete mainUI;
    delete app;
    delete sLogHandler;
    sLogHandler = nullptr;

    return ret;
}

