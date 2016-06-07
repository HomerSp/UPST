#include <QtGlobal>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QFile>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QSettings>
#include <QTextStream>
#include <QThread>

#include "runguard.h"
#include "utils/computerutils.h"
#include "utils/fileutils.h"
#include "ui/updaterui.h"
#include "ui/ui.h"

#ifdef Q_OS_WIN
#include "utils/winutils.h"
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

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationDomain("ultimobile.net");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    sLogHandler = new Log::LogHandler("upst.log");

    qInstallMessageHandler(&logMessageHandler);

    QGuiApplication *app = new QGuiApplication(argc, argv);
    app->setWindowIcon(QIcon(":/res/images/icon.svg"));

    QStringList args = QCoreApplication::arguments();

    bool isUpdater = (args.size() == 2 && args.at(1) == "updater");
    bool isUpdate = (args.size() == 4 && args.at(1) == "update");

    RunGuard runGuard("UPST_main_87f5a82db5098c594efe3b3cefe67cd1b38dcd6c");
    if(isUpdater || isUpdate) {
        // Wait for the other process to finish.
        int retries = 0;
        while(!runGuard.tryToRun()) {
            QThread::sleep(1);
            retries++;
            if(retries >= 10) {
                delete app;
                delete sLogHandler;
                sLogHandler = nullptr;

                return -1;
            }
        }
    } else if(!runGuard.tryToRun()) {
        delete app;
        delete sLogHandler;
        sLogHandler = nullptr;

        return 0;
    }

    Utils::ComputerUtils::init();
    Utils::ComputerUtils::enableIntelHack();

    bool updateFailed = false;
    if(isUpdate) {
        // Wait for the Updater to finish.
        {
            RunGuard updaterGuard("UPST_updater_87f5a82db5098c594efe3b3cefe67cd1b38dcd6c");
            while(!updaterGuard.tryToRun()) {
                QThread::sleep(1);
            }
        }

        updateFailed = (args.at(2) != PROG_BUILDTIME);
        QString updatePath = args.at(3);

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
                    qDebug()<<"Removing"<<file;
                    if(!QFile::remove(updatePath + "/" + file)) {
                        retry = true;
                        break;
                    }
                }

                if(retry) {
                    retries++;
                    QThread::sleep(1);
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

    int ret = 0;
    if(isUpdater) {
        UI::UpdaterUI* updaterUI = new UI::UpdaterUI(*app);

        ret = app->exec();

        delete updaterUI;
    } else if(args.size() == 4 && args.at(1) == "taskcreate") {
#ifdef Q_OS_WIN
        QString user = qgetenv("USER");
        if(user.isEmpty()) {
            user = qgetenv("USERNAME");
        }

        Utils::WinUtils::executeElevated(QCoreApplication::applicationDirPath() + "/Updater.exe", "taskinit \"" + user + "\"", QCoreApplication::applicationDirPath());
        Utils::WinUtils::enableUpdaterTask(args.at(2), args.at(3));
        Utils::WinUtils::executeElevated(QCoreApplication::applicationDirPath() + "/Updater.exe", "taskelevate \"" + args.at(2) + "\" \"" + args.at(3) + "\"", QCoreApplication::applicationDirPath());
#endif

        ret = 0;
    } else {
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

        ret = app->exec();

        delete mainUI;
    }

    delete app;
    delete sLogHandler;
    sLogHandler = nullptr;

    return ret;
}

