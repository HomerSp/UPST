#include <QtGlobal>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QIcon>
#include <QSettings>
#include <QThread>
#include <QTemporaryDir>
#include <QProcess>

#include "loghandler.h"
#include "runguard.h"
#include "utils/computerutils.h"
#include "utils/fileutils.h"
#ifdef Q_OS_WIN
#include "utils/winutils.h"
#endif
#include "updateworker.h"

static Log::LogHandler *sLogHandler = nullptr;

void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if(sLogHandler != nullptr) {
        sLogHandler->addLog(type, context, msg);
    }
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(common);

    RunGuard runGuard("UPST_updater_87f5a82db5098c594efe3b3cefe67cd1b38dcd6c");
    if(!runGuard.tryToRun()) {
        return 0;
    }

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationDomain("ultimobile.net");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    sLogHandler = new Log::LogHandler("updater.log");

    qInstallMessageHandler(&logMessageHandler);

    QCoreApplication *app = new QCoreApplication(argc, argv);

    int ret = 0;

    QStringList args = QCoreApplication::arguments();
    if(args.size() >= 2 && args.at(1) == "create") {
        QList<QString> files;
        if(args.size() == 2) {
            Utils::FileUtils::listFiles(files, QCoreApplication::applicationDirPath());
        } else {
            for(int i = 2; i < args.size(); i++) {
                files.append(args.at(i));
            }
        }

        QFile outputFile("out.upz");
        outputFile.remove();

#ifdef Q_OS_WIN
        if(!files.contains("UPST.exe")) {
#else
        if(!files.contains("UPST")) {
#endif
            return -1;
        }

        uint64_t buildTime = 0;
        QString buildVersion = "";
        bool buildTesting = false;
        {
            QTemporaryDir dir;
            QFile buildFile(dir.path() + "/build.ini");

            QStringList args;
            args << "build" << buildFile.fileName();

#ifdef Q_OS_WIN
            QProcess::execute(QCoreApplication::applicationDirPath() + "/UPST.exe", args);
#else
            QProcess::execute(QCoreApplication::applicationDirPath() + "/UPST", args);
#endif

            QSettings buildSettings(buildFile.fileName(), QSettings::IniFormat);
            buildTime = buildSettings.value("time").toULongLong();
            buildVersion = buildSettings.value("version").toString();
            buildTesting = buildSettings.value("testing", false).toBool();
        }

        QByteArray data;
        data.append('U');
        data.append('P');
        data.append('Z');

        uint16_t updaterVersion = static_cast<uint16_t>(QString(UPDATER_VERSION).toUInt());
        data.append(static_cast<uint8_t>((updaterVersion) & 0xFF));
        data.append(static_cast<uint8_t>((updaterVersion >> 8) & 0xFF));

        data.append(static_cast<uint8_t>(buildTesting & 0xFF));

        data.append(static_cast<uint8_t>((buildTime) & 0xFF));
        data.append(static_cast<uint8_t>((buildTime >> 8) & 0xFF));
        data.append(static_cast<uint8_t>((buildTime >> 16) & 0xFF));
        data.append(static_cast<uint8_t>((buildTime >> 24) & 0xFF));
        data.append(static_cast<uint8_t>((buildTime >> 32) & 0xFF));
        data.append(static_cast<uint8_t>((buildTime >> 40) & 0xFF));
        data.append(static_cast<uint8_t>((buildTime >> 48) & 0xFF));
        data.append(static_cast<uint8_t>((buildTime >> 56) & 0xFF));

        data.append(buildVersion.toUtf8());
        data.append('\0');

        foreach(QString fileName, files) {
            if(fileName == "out.upz") {
                continue;
            }

            QFileInfo file(QCoreApplication::applicationDirPath() + "/" + fileName);
            QFile inFile(file.absoluteFilePath());
            if(!inFile.open(QIODevice::ReadOnly)) {
                continue;
            }

            data.append(fileName.toUtf8());
            data.append('\0');

            qint64 size = file.size();
            data.append(static_cast<uint8_t>((size) & 0xFF));
            data.append(static_cast<uint8_t>((size >> 8) & 0xFF));
            data.append(static_cast<uint8_t>((size >> 16) & 0xFF));
            data.append(static_cast<uint8_t>((size >> 24) & 0xFF));
            data.append(static_cast<uint8_t>((size >> 32) & 0xFF));
            data.append(static_cast<uint8_t>((size >> 40) & 0xFF));
            data.append(static_cast<uint8_t>((size >> 48) & 0xFF));
            data.append(static_cast<uint8_t>((size >> 56) & 0xFF));

            QByteArray inData;

            QByteArray buffer;
            do {
                buffer = inFile.read(1048576);
                if(buffer.size() == 0) {
                    break;
                }

                inData.append(buffer);
            } while(buffer.size() > 0);

            QByteArray compressedData = ::qCompress(inData);

            qint64 compressedSize = compressedData.size();
            data.append(static_cast<uint8_t>((compressedSize) & 0xFF));
            data.append(static_cast<uint8_t>((compressedSize >> 8) & 0xFF));
            data.append(static_cast<uint8_t>((compressedSize >> 16) & 0xFF));
            data.append(static_cast<uint8_t>((compressedSize >> 24) & 0xFF));
            data.append(static_cast<uint8_t>((compressedSize >> 32) & 0xFF));
            data.append(static_cast<uint8_t>((compressedSize >> 40) & 0xFF));
            data.append(static_cast<uint8_t>((compressedSize >> 48) & 0xFF));
            data.append(static_cast<uint8_t>((compressedSize >> 56) & 0xFF));

            data.append(compressedData);

            inFile.close();
        }

        if(outputFile.open(QIODevice::WriteOnly)) {
            outputFile.write(data);
            outputFile.flush();
            outputFile.close();
        }
    } else if(args.size() == 2 && args.at(1) == "task") {
        qInfo()<<"Starting UPST Updater"<<PROG_VERSION<<"at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

        QSettings settings(QCoreApplication::applicationDirPath() + "/Updater.ini", QSettings::IniFormat);

        QThread* thread = new QThread;

        UI::UpdateWorker* worker = new UI::UpdateWorker(settings.value("id").toString(), settings.value("time").toULongLong(), settings.value("path").toString());
        worker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, worker, &UI::UpdateWorker::process);
        QObject::connect(worker, &UI::UpdateWorker::finished, thread, &QThread::quit);
        QObject::connect(worker, &UI::UpdateWorker::finished, worker, &QThread::deleteLater);
        QObject::connect(thread, &QThread::finished, app, &QCoreApplication::quit);
        QObject::connect(thread, &QThread::finished, worker, &UI::UpdateWorker::deleteLater);

        thread->start();

        ret = app->exec();
    } else if(args.size() == 3 && args.at(1) == "taskinit") {
#ifdef Q_OS_WIN
        Utils::WinUtils::initTaskScheduler(args.at(2));

        QSettings upstSettings(QCoreApplication::applicationDirPath() + "/UPST.ini", QSettings::IniFormat);
        upstSettings.setValue("Updater/UseTask", 1);
        upstSettings.sync();
#endif
        ret = 0;
    } else if(args.size() == 4 && args.at(1) == "taskelevate") {
        Utils::WinUtils::elevateUpdaterTask(args.at(2), args.at(3));

        ret = 0;
    } else {
        ret = -1;
    }

    delete app;
    delete sLogHandler;
    sLogHandler = nullptr;

    return ret;
}

