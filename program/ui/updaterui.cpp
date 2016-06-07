#include <QDebug>
#include <QQmlContext>
#include <QQmlProperty>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QThread>
#include <QLocalSocket>
#include <QDataStream>

#include "updater/updater.h"
#include "utils/fileutils.h"
#include "updaterui.h"

UI::UpdaterUI::UpdaterUI(const QGuiApplication& app)
    : QObject(),
      mApp(app),
      mHaveClient(false)
{
    QSettings updaterSettings(QCoreApplication::applicationDirPath() + "/Updater.ini", QSettings::IniFormat);
    mInstallDir = updaterSettings.value("path").toString();
    mUpdateTime = updaterSettings.value("time").toULongLong();

    mStatusServer = new QLocalServer(this);
    if(!mStatusServer->listen("upst_updater_" + QString::number(mUpdateTime))) {
        qCritical()<<"Failed to open local server"<<mStatusServer->errorString();
        mApp.quit();
        return;
    }

    QObject::connect(mStatusServer, &QLocalServer::newConnection, this, &UI::UpdaterUI::updaterConnection);

    mEngine = new QQmlApplicationEngine();
    mEngine->rootContext()->setContextProperty("programVersion", QString(PROG_VERSION));
    mEngine->rootContext()->setContextProperty("qtVersion", QString(QT_VERSION_STR));

    QObject::connect(mEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    mEngine->load(QUrl(QStringLiteral("qrc:/res/qml/updater.qml")));

#ifdef Q_OS_WIN
    QSettings upstSettings(QCoreApplication::applicationDirPath() + "/UPST.ini", QSettings::IniFormat);
    if(upstSettings.value("Updater/UseTask", 0).toInt() == 1) {
        QStringList argumentsList;
        argumentsList << "/run" << "/tn" << TASK_NAME;

        QProcess::startDetached("schtasks", argumentsList, QCoreApplication::applicationDirPath());
    } else {
        QStringList argumentsList;
        argumentsList << "task";

        QProcess::startDetached(QCoreApplication::applicationDirPath() + "/Updater.exe", argumentsList, QCoreApplication::applicationDirPath());
    }
#else
    QProcess::startDetached(QCoreApplication::applicationDirPath() + "/Updater", argumentsList, QCoreApplication::applicationDirPath());
#endif

    QTimer::singleShot(10000, this, &UI::UpdaterUI::updaterTimeout);
}

UI::UpdaterUI::~UpdaterUI() {
    delete mEngine;
}

void UI::UpdaterUI::updaterTimeout() {
    if(!mHaveClient) {
        qCritical()<<"Timed out waiting for updater to start!";
    }
}

void UI::UpdaterUI::updaterConnection() {
    mHaveClient = true;

    QLocalSocket* client = mStatusServer->nextPendingConnection();
    connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);
    connect(client, &QIODevice::readyRead, this, &UI::UpdaterUI::updaterDataReady);

    QByteArray block;
    block.append(static_cast<uint8_t>(0x1));
    block.append('\n');

    client->write(block);
    client->flush();
}

void UI::UpdaterUI::updaterDataReady() {
    QLocalSocket* client = static_cast<QLocalSocket*>(sender());

    QByteArray data = client->readAll();
    if(data.size() == 0) {
        return;
    }

    if(mBufferedData.size() > 0) {
        data = mBufferedData + data;
    }

    if(data.size() < 2) {
        mBufferedData = data;
        return;
    }

    while(data.size() > 0) {
        uint8_t type = data.at(0);

        if(type == Updater::UpdateStatusDownloadProgress || type == Updater::UpdateStatusInstallProgress) {
            // type + qint64 * 2 + \n
            if(data.size() < 18) {
                mBufferedData = data;
                return;
            }
        }

        switch(type) {
        case Updater::UpdateStatusDownload: {
            updateStatus("download");
            break;
        }
        case Updater::UpdateStatusInstall: {
            updateStatus("install");
            break;
        }
        case Updater::UpdateStatusDownloadProgress:
        case Updater::UpdateStatusInstallProgress: {
            qint64 received = static_cast<qint64>(data.at(0) & 0xFF)
                | static_cast<qint64>(data.at(1) & 0xFF) << 8
                | static_cast<qint64>(data.at(2) & 0xFF) << 16
                | static_cast<qint64>(data.at(3) & 0xFF) << 24
                | static_cast<qint64>(data.at(4) & 0xFF) << 32
                | static_cast<qint64>(data.at(5) & 0xFF) << 40
                | static_cast<qint64>(data.at(6) & 0xFF) << 48
                | static_cast<qint64>(data.at(7) & 0xFF) << 56;

            qint64 total = static_cast<qint64>(data.at(8) & 0xFF)
                    | static_cast<qint64>(data.at(9) & 0xFF) << 8
                    | static_cast<qint64>(data.at(10) & 0xFF) << 16
                    | static_cast<qint64>(data.at(11) & 0xFF) << 24
                    | static_cast<qint64>(data.at(12) & 0xFF) << 32
                    | static_cast<qint64>(data.at(13) & 0xFF) << 40
                    | static_cast<qint64>(data.at(14) & 0xFF) << 48
                    | static_cast<qint64>(data.at(15) & 0xFF) << 56;

            if(type == Updater::UpdateStatusDownloadProgress) {
                downloadProgress(received, total);
            } else  {
                installProgress(received, total);
            }

            break;
        }
        case Updater::UpdateStatusFinished: {
            installFinished();
            break;
        }
        case Updater::UpdateStatusError: {
            updateStatus("error");
            break;
        }
        default: {
            qWarning()<<"Unknown updater command"<<type;
        }
        }

        data.remove(0, 1);
        if(type == Updater::UpdateStatusDownloadProgress || type == Updater::UpdateStatusInstallProgress) {
            data.remove(0, sizeof(qint64) * 2);
        }
        data.remove(0, 1);
    }
}

void UI::UpdaterUI::downloadProgress(quint64 received, quint64 total) {
    if(total == 0) {
        return;
    }

    QObject* rootObject = mEngine->rootObjects().first();

    int p = (received / (float)total) * 50.0f;

    QObject* progressObject = rootObject->findChild<QObject*>("updatingProgress");
    progressObject->setProperty("value", p);
}

void UI::UpdaterUI::installProgress(quint64 received, quint64 total) {
    if(total == 0) {
        return;
    }

    QObject* rootObject = mEngine->rootObjects().first();

    int p = (received / (float)total) * 50.0f;

    QObject* progressObject = rootObject->findChild<QObject*>("updatingProgress");
    progressObject->setProperty("value", 50 + p);
}

void UI::UpdaterUI::installFinished() {
    QString upstFile = mInstallDir + "/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    QStringList args;
    args << "update" << QString::number(mUpdateTime) << QCoreApplication::applicationDirPath();

    QProcess::startDetached(upstFile, args, mInstallDir);

    mApp.quit();
}

void UI::UpdaterUI::updateStatus(const QString &status) {
    QObject* rootObject = mEngine->rootObjects().first();
    rootObject->setProperty("processStatus", status);
}
