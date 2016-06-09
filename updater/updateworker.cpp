#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>

#include "web/webutils.h"

#include "updateworker.h"

UI::UpdateWorker::UpdateWorker(const QString& id, uint64_t updateTime, const QString& installDir)
    : mID(id),
      mUpdateTime(updateTime),
      mInstallDir(installDir)
{
    mSocket = new QLocalSocket(this);
}

UI::UpdateWorker::~UpdateWorker() {
    mSocket->disconnectFromServer();
}

void UI::UpdateWorker::process() {
    mSocket->connectToServer("upst_updater_" + QString::number(mUpdateTime));

    QByteArray data;
    do {
        if(!mSocket->waitForReadyRead()) {
            updateStatus(Updater::UpdateStatusError);
            emit finished();
            return;
        }

        QByteArray buf = mSocket->readAll();
        data += buf;
    } while(data.size() < 2);

    if(data.at(0) != 0x1) {
        updateStatus(Updater::UpdateStatusError);
        emit finished();
        return;
    }

    QByteArray output;
    QHash<QString, QString> headers;
    QString postData = "i=" + mID;

    Web::WebDownloadStatus status;
    QObject::connect(&status, &Web::WebDownloadStatus::progress, this, &UI::UpdateWorker::downloadProgress);

    updateStatus(Updater::UpdateStatusDownload);

    if(!Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/download_update.php"), output, headers, postData, &status)) {
        updateStatus(Updater::UpdateStatusError);
        emit finished();
        return;
    }

    updateStatus(Updater::UpdateStatusInstall);

    updateStatus(Updater::UpdateStatusInstallProgress, 0, output.size());

    qint64 i = 0;
    uint16_t updateVersion = 0;
    bool buildTesting = false;
    uint64_t buildTime = 0;
    QString buildVersion = "";
    if(output.size() >= 5) {
        if(output.at(i) == 'U' && output.at(i + 1) == 'P' && output.at(i + 2) == 'Z') {
            i += 3;

            updateVersion = static_cast<uint16_t>(output.at(i) & 0xFF)
                | static_cast<uint16_t>(output.at(i + 1) & 0xFF) << 8;

            i += sizeof(updateVersion);

            buildTesting = output.at(i) == 0x01;
            i++;

            buildTime = static_cast<quint64>(output.at(i) & 0xFF)
                    | static_cast<quint64>(output.at(i + 1) & 0xFF) << 8
                    | static_cast<quint64>(output.at(i + 2) & 0xFF) << 16
                    | static_cast<quint64>(output.at(i + 3) & 0xFF) << 24
                    | static_cast<quint64>(output.at(i + 4) & 0xFF) << 32
                    | static_cast<quint64>(output.at(i + 5) & 0xFF) << 40
                    | static_cast<quint64>(output.at(i + 6) & 0xFF) << 48
                    | static_cast<quint64>(output.at(i + 7) & 0xFF) << 56;

            i += sizeof(buildTime);

            buildVersion = QString::fromUtf8(output.constData() + i);
            i += buildVersion.toUtf8().size() + 1;

#ifndef TESTING_MODE
            if(buildTesting) {
                qCritical()<<"Trying to update to a testing build from a release build, failing...";

                updateStatus(Updater::UpdateStatusError);
                emit finished();
                return;
            }
#else
            if(!buildTesting) {
                qCritical()<<"Trying to update to a release build from a testing build, failing...";

                updateStatus(Updater::UpdateStatusError);
                emit finished();
                return;
            }
#endif

            if(updateVersion > QString(UPDATER_VERSION).toInt()) {
                qCritical()<<"Updater package version"<<updateVersion<<"is higher than supported"<<UPDATER_VERSION<<", failing...";

                updateStatus(Updater::UpdateStatusError);
                emit finished();
                return;
            }
        }
    }

    updateStatus(Updater::UpdateStatusInstallProgress, i, output.size());

    while(i < output.size()) {
        QString name = QString::fromUtf8(output.constData() + i);
        i += name.toUtf8().size() + 1;

        quint64 originalSize = static_cast<quint64>(output.at(i) & 0xFF)
            | static_cast<quint64>(output.at(i + 1) & 0xFF) << 8
            | static_cast<quint64>(output.at(i + 2) & 0xFF) << 16
            | static_cast<quint64>(output.at(i + 3) & 0xFF) << 24
            | static_cast<quint64>(output.at(i + 4) & 0xFF) << 32
            | static_cast<quint64>(output.at(i + 5) & 0xFF) << 40
            | static_cast<quint64>(output.at(i + 6) & 0xFF) << 48
            | static_cast<quint64>(output.at(i + 7) & 0xFF) << 56;

        Q_UNUSED(originalSize);

        i += 8;

        quint64 compressedSize = static_cast<quint64>(output.at(i) & 0xFF)
            | static_cast<quint64>(output.at(i + 1) & 0xFF) << 8
            | static_cast<quint64>(output.at(i + 2) & 0xFF) << 16
            | static_cast<quint64>(output.at(i + 3) & 0xFF) << 24
            | static_cast<quint64>(output.at(i + 4) & 0xFF) << 32
            | static_cast<quint64>(output.at(i + 5) & 0xFF) << 40
            | static_cast<quint64>(output.at(i + 6) & 0xFF) << 48
            | static_cast<quint64>(output.at(i + 7) & 0xFF) << 56;

        i += 8;

        QByteArray compressedData = output.mid(i, compressedSize);
        QByteArray data = ::qUncompress(compressedData);

        QDir().mkpath(QFileInfo(mInstallDir + "/" + name).absoluteDir().absolutePath());

        QFile outputFile(mInstallDir + "/" + name);
        qDebug()<<"Extracting"<<outputFile.fileName();
        if(outputFile.open(QIODevice::WriteOnly)) {
            outputFile.write(data);
            outputFile.flush();
            outputFile.close();
        } else {
            qWarning()<<"Failed to update"<<name;
        }

        i += compressedSize;

        updateStatus(Updater::UpdateStatusInstallProgress, i, output.size());
    }

    updateStatus(Updater::UpdateStatusFinished);

    emit finished();
}

void UI::UpdateWorker::downloadProgress(qint64 received, qint64 total) {
    updateStatus(Updater::UpdateStatusDownloadProgress, static_cast<quint64>(received), static_cast<quint64>(total));
}

void UI::UpdateWorker::updateStatus(Updater::UpdateStatus status, quint64 received, quint64 total) {
    QByteArray block;
    block.append(static_cast<uint8_t>(status));

    if(status == Updater::UpdateStatusDownloadProgress || status == Updater::UpdateStatusInstallProgress) {
        block.append(static_cast<uint8_t>((received) & 0xFF));
        block.append(static_cast<uint8_t>((received >> 8) & 0xFF));
        block.append(static_cast<uint8_t>((received >> 16) & 0xFF));
        block.append(static_cast<uint8_t>((received >> 24) & 0xFF));
        block.append(static_cast<uint8_t>((received >> 32) & 0xFF));
        block.append(static_cast<uint8_t>((received >> 40) & 0xFF));
        block.append(static_cast<uint8_t>((received >> 48) & 0xFF));
        block.append(static_cast<uint8_t>((received >> 56) & 0xFF));

        block.append(static_cast<uint8_t>((total) & 0xFF));
        block.append(static_cast<uint8_t>((total >> 8) & 0xFF));
        block.append(static_cast<uint8_t>((total >> 16) & 0xFF));
        block.append(static_cast<uint8_t>((total >> 24) & 0xFF));
        block.append(static_cast<uint8_t>((total >> 32) & 0xFF));
        block.append(static_cast<uint8_t>((total >> 40) & 0xFF));
        block.append(static_cast<uint8_t>((total >> 48) & 0xFF));
        block.append(static_cast<uint8_t>((total >> 56) & 0xFF));
    }

    block.append('\n');

    mSocket->write(block);
    mSocket->flush();
}
