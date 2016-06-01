#include <QDebug>
#include <QSettings>
#include <QThread>
#include <QTemporaryDir>

#include "utils/computerutils.h"
#include "utils/fileutils.h"
#include "web/webutils.h"

#include "uiworker.h"

UI::Worker::UIWorker::UIWorker(bool updateFailed)
    :   mThread(new QThread),
        mRunning(true),
        mUpdateFailed(updateFailed)
{
    moveToThread(mThread);

    connect(mThread, &QThread::started, this, &UIWorker::process);
    connect(this, &UIWorker::finished, mThread, &QThread::quit);
    connect(this, &UIWorker::finished, this, &QThread::deleteLater);
    connect(mThread, &QThread::finished, this, &UIWorker::deleteLater);
}

UI::Worker::UIWorker::~UIWorker() {

}

void UI::Worker::UIWorker::addDevicesListUpdate() {
    qDebug()<<"addDevicesListUpdate";

    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDevicesListUpdate, nullptr));
    mWaitCondition.wakeAll();
}

void UI::Worker::UIWorker::addDeviceProvisionTracking(Serial::SerialDevice* device, bool error, const QString& log) {
    qDebug()<<"addDeviceProvisionTracking";

    ProvisionTrackingItem* item = new ProvisionTrackingItem();
    item->device = device;
    item->error = error;
    item->log = log;
    item->serialNumber = Utils::ComputerUtils::serialNumber();

    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceProvisionTracking, item));
    mWaitCondition.wakeAll();
}

void UI::Worker::UIWorker::addLogin(const QString &username, const QString &password) {
    LoginItem* item = new LoginItem();
    item->username = username;
    item->password = password;
    item->token = "";

    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeLogin, item));
    mWaitCondition.wakeAll();
}

void UI::Worker::UIWorker::addLoginCheck(const QString& token) {
    LoginItem* item = new LoginItem();
    item->username = "";
    item->password = "";
    item->token = token;

    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeLogin, item));
    mWaitCondition.wakeAll();
}

void UI::Worker::UIWorker::start() {
    mThread->start();
}

void UI::Worker::UIWorker::stop() {
    QMutexLocker stopLock(&mStoppedMutex);

    mRunning = false;

    // TODO: Make this wait until we are actually in the wait condition.
    mWaitCondition.wakeAll();

    // Wait for the worker to finish.
    QMutexLocker runningLock(&mRunningMutex);

    QMutexLocker lock(&mWorkMutex);
    for(QList<QPair<WorkType, void*> >::iterator i = mWorkItems.begin(); i != mWorkItems.end(); i++) {
        if((*i).first == WorkTypeDeviceProvisionTracking) {
            delete static_cast<ProvisionTrackingItem*>((*i).second);
        } else if((*i).first == WorkTypeLogin) {
            delete static_cast<LoginItem*>((*i).second);
        }
    }

    mWorkItems.clear();
}

void UI::Worker::UIWorker::process() {
    while(mRunning.load()) {
        bool waitForNextCommand = true;
        {
            QMutexLocker runningLock(&mRunningMutex);

            qDebug()<<"UIWorker: process";

            int size = 0;
            do {
                WorkType type;
                void* data = nullptr;

                {
                    QMutexLocker locker(&mWorkMutex);
                    size = mWorkItems.size();
                    if(size == 0 || !mRunning.load()) {
                        break;
                    }

                    type = (*mWorkItems.begin()).first;
                    data = (*mWorkItems.begin()).second;
                }

                bool unknownCommand = false;
                switch(type) {
                case WorkTypeDevicesListUpdate: {
                    processDevicesListUpdate();
                    break;
                }
                case WorkTypeDeviceProvisionTracking: {
                    processDeviceProvisionTracking(static_cast<ProvisionTrackingItem*>(data));
                    break;
                }
                case WorkTypeLogin: {
                    processLogin(static_cast<LoginItem*>(data));
                    break;
                }
                default:
                    unknownCommand = true;
                }

                if(unknownCommand) {
                    waitForNextCommand = false;
                    break;
                }

                {
                    QMutexLocker locker(&mWorkMutex);
                    if(mWorkItems.size() > 0) {
                        mWorkItems.erase(mWorkItems.begin());
                    }
                }

                waitForNextCommand = false;
            } while(size > 0);
        }

        // Wait until we have a new process item
        if(waitForNextCommand && mRunning.load()) {
            emit statusChange("");

            qDebug()<<"UIWorker: Sleeping until next command";
            QMutexLocker locker(&mWakeMutex);
            mWaitCondition.wait(&mWakeMutex);
        }
    }

    // Wait for the stop handler to finish before returning
    QMutexLocker stopLock(&mStoppedMutex);
    qDebug()<<"UIWorker: finished";

    emit finished();
}

void UI::Worker::UIWorker::processDevicesListUpdate() {
    qDebug()<<"processDevicesListUpdate";

    emit statusChange("Updating devices list");

    QString token = QSettings().value("user/token").toString();

    QByteArray output;
    QHash<QString, QString> headers;
    headers.insert("U-Token", token);

    if(Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/devices.php"), output, headers)) {
        Serial::SerialDeviceConfig* config = new Serial::SerialDeviceConfig(QString(output));
        emit devicesListChanged(config->isValid(), config);
    } else {
         emit devicesListChanged(false);
    }

    emit statusChange("");
}

bool UI::Worker::UIWorker::processDeviceProvisionTracking(ProvisionTrackingItem* item) {
    QString userToken = QSettings().value("user/token").toString();

    QByteArray output;
    QHash<QString, QString> headers;
    headers.insert("U-Token", userToken);

    QJsonObject deviceObj;
    deviceObj.insert("id", item->device->id());
    deviceObj.insert("min", item->device->newMinStr());
    deviceObj.insert("mdn", item->device->newMdnStr());
    deviceObj.insert("uniqueID", QString(QCryptographicHash::hash(item->device->imeiStr().toLatin1(), QCryptographicHash::Sha256).toHex()));

    QJsonObject computerObj;
    computerObj.insert("serial", item->serialNumber);

    QJsonObject obj;
    obj.insert("device", QJsonValue(deviceObj));
    obj.insert("computer", QJsonValue(computerObj));

    if(item->error) {
        obj.insert("log", item->log);
    }

    QJsonDocument doc(obj);
    QString postData = doc.toJson();

    bool ret = Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/tracking.php"), output, headers, postData);

    delete item;
    return ret;
}

void UI::Worker::UIWorker::processLogin(LoginItem* item) {
    QByteArray output;
    QHash<QString, QString> headers;
    QString postData = "";

    if(item->token.size() > 0) {
        headers.insert("U-Token", item->token);
    } else {
        postData = "u=" + QString(QCryptographicHash::hash(item->username.toLatin1(), QCryptographicHash::Sha256).toHex()) + "&p=" + QString(QCryptographicHash::hash(item->password.toLatin1(), QCryptographicHash::Sha256).toHex());
    }

    if(!Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/login.php"), output, headers, postData)) {
        emit loginStatus(false);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(output);
        if(doc.isObject() && doc.object().contains("display_name")) {
            QString displayName = doc.object()["display_name"].toString();

            QSettings settings;
            settings.setValue("user/display_name", displayName);
            settings.sync();

            if(item->token.size() > 0) {
                if(!processLoginCheckUpdate(item->token)) {
                    emit loginStatus(true);
                }
            } else {
                if(doc.object().contains("token")) {
                    QString token = doc.object()["token"].toString();

                    settings.setValue("user/token", token);
                    settings.sync();

                    if(!processLoginCheckUpdate(token)) {
                        emit loginStatus(true);
                    }
                } else {
                    emit loginStatus(false);
                }
            }
        } else {
            emit loginStatus(false);
        }
    }

    delete item;
}

bool UI::Worker::UIWorker::processLoginCheckUpdate(const QString& token) {
    emit updateCheck();

    if(mUpdateFailed) {
        return true;
    }

    QByteArray output;
    QHash<QString, QString> headers;
    headers.insert("U-Token", token);

    QString postData = "t=" + QString(PROG_BUILDTIME) + "&u=" + QString(UPDATER_VERSION);
#ifdef TESTING_MODE
    postData += "&d=1";
#endif

    if(!Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/check_update.php"), output, headers, postData)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(output);
    if(doc.isObject()) {
        QJsonObject obj = doc.object();
        if(obj.contains("id") && obj.contains("version") && obj.contains("time")) {
            QTemporaryDir updaterDir;
            if(!updaterDir.isValid()) {
                return false;
            }

            QString sourcePath = QCoreApplication::applicationDirPath();
            QString targetPath = updaterDir.path();

            qDebug()<<"Copying"<<sourcePath<<"to"<<targetPath;

            QList<QString> files;
            Utils::FileUtils::listFiles(files, sourcePath);

            foreach(QString fileName, files) {
                QFile source(sourcePath + "/" + fileName);
                QFileInfo target(targetPath + "/" + fileName);

                QDir().mkpath(target.absoluteDir().absolutePath());
                source.copy(target.absoluteFilePath());
            }

            updaterDir.setAutoRemove(false);

            emit updateAvailable(updaterDir.path(), obj["id"].toString(), obj["version"].toString(), QDateTime::fromTime_t(obj["time"].toInt()));

            return true;
        }
    }

    return false;
}
