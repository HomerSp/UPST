#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "web/webutils.h"

#include "updateworker.h"

UI::UpdateWorker::UpdateWorker(const QString& id, const QString& installDir)
    : mID(id),
      mInstallDir(installDir)
{

}

void UI::UpdateWorker::process() {
    QByteArray output;
    QHash<QString, QString> headers;
    QString postData = "i=" + mID;

    Web::WebDownloadStatus status;
    QObject::connect(&status, &Web::WebDownloadStatus::progress, this, &UI::UpdateWorker::downloadProgress);

    emit updateStatus("download");

    if(!Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/download_update.php"), output, headers, postData, &status)) {
        emit updateStatus("error");
        return;
    }

    emit updateStatus("install");

    emit installProgress(0, output.size());

    qint64 i = 0;
    while(i < output.size()) {
        uint16_t nameSize = static_cast<uint16_t>((output.at(i + 1) & 0xFF) << 8) | static_cast<uint16_t>(output.at(i) & 0xFF);
        i += 2;

        QString name = output.mid(i, nameSize);
        i += nameSize;

        quint64 originalSize = static_cast<quint64>(output.at(i) & 0xFF)
            | static_cast<quint64>(output.at(i + 1) & 0xFF) << 8
            | static_cast<quint64>(output.at(i + 2) & 0xFF) << 16
            | static_cast<quint64>(output.at(i + 3) & 0xFF) << 24
            | static_cast<quint64>(output.at(i + 4) & 0xFF) << 32
            | static_cast<quint64>(output.at(i + 5) & 0xFF) << 40
            | static_cast<quint64>(output.at(i + 6) & 0xFF) << 48
            | static_cast<quint64>(output.at(i + 7) & 0xFF) << 56;

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

        qDebug()<<"name"<<name<<originalSize;

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

        emit installProgress(i, output.size());
    }
}
