#include <QtGlobal>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

#include "utils/fileutils.h"
#include "ui/ui.h"

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

    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/updater.log");
    file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
    QTextStream stream(&file);
    stream << data;
    stream.flush();
    file.close();

    QTextStream(stdout) << data;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(common);

    qInstallMessageHandler(&logMessageHandler);

    QGuiApplication::setApplicationName("UPST");
    QGuiApplication::setOrganizationDomain("ultimobile.net");
    QGuiApplication::setOrganizationName("Ultimobile");

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QGuiApplication app(argc, argv);

    QStringList args = QCoreApplication::arguments();
    if(args.size() == 2 && args.at(1) == "create") {
        QList<QString> files;
        Utils::FileUtils::listFiles(files, QCoreApplication::applicationDirPath());

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
        qInfo()<<"Starting UPST Updater"<<PROG_VERSION<<"at"<<QDateTime::currentDateTime().toString(Qt::ISODate);

        UI::MainUI mainUI(app, args.at(1), args.at(3));
        return app.exec();
    }

    return -1;
}

