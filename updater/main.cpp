#include <QtGlobal>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

#include "loghandler.h"
#include "runguard.h"
#include "utils/fileutils.h"
#ifdef Q_OS_WIN
#include "utils/winutils.h"
#endif
#include "ui/ui.h"

Log::LogHandler sLogHandler("updater.log");

void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    sLogHandler.addLog(type, context, msg);
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(common);

    RunGuard runGuard("UPST_updater_87f5a82db5098c594efe3b3cefe67cd1b38dcd6c");
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

        QByteArray data;
        foreach(QString fileName, files) {
            if(fileName == "out.umz") {
                continue;
            }

            QFileInfo file(QCoreApplication::applicationDirPath() + "/" + fileName);
            QFile inFile(file.absoluteFilePath());
            if(!inFile.open(QIODevice::ReadOnly)) {
                continue;
            }

            data.append(static_cast<uint8_t>((fileName.length()) & 0xFF));
            data.append(static_cast<uint8_t>((fileName.length() >> 8) & 0xFF));
            data.append(fileName.toLatin1());

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

        QFile outputFile("out.umz");
        if(outputFile.open(QIODevice::WriteOnly)) {
            outputFile.write(data);
            outputFile.flush();
            outputFile.close();
        }

        return 0;
    }
    if(args.size() == 4) {
        qInfo()<<"Starting UPST Updater"<<PROG_VERSION<<"at"<<QDateTime::currentDateTime().toString(Qt::ISODate)<<"arg 1"<<args.at(1)<<"arg 2"<<args.at(3);

        UI::MainUI mainUI(app, args.at(1), args.at(3));
        return app.exec();
    }

    return -1;
}

