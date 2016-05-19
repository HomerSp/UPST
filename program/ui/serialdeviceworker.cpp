#include <QDebug>
#include <QSettings>
#include <QThread>

#include "utils/fileutils.h"
#include "web/webutils.h"

#include "ui.h"
#include "serialdeviceworker.h"

UI::SerialCommandItem::SerialCommandItem(Serial::SerialDevice* device)
    : mDevice(device)
{

}

UI::SerialCommandItem::~SerialCommandItem() {
    foreach(Serial::SerialCommand* cmd, mCmds) {
        delete cmd;
    }
}

void UI::SerialCommandItem::addItem(Serial::SerialCommand *cmd) {
    mCmds.append(cmd);
}

void UI::SerialCommandItem::process() {
    foreach(Serial::SerialCommand* cmd, mCmds) {
        cmd->execute();
    }

    emit finished();
}

UI::SerialDeviceWorker::SerialDeviceWorker()
    :   mRunning(true)
{
    mDeviceConfig = new Serial::SerialDeviceConfig();
}

UI::SerialDeviceWorker::~SerialDeviceWorker() {
    delete mDeviceConfig;
}

void UI::SerialDeviceWorker::addDevicesListUpdate() {
    qDebug()<<"addDevicesListUpdate";

    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDevicesListUpdate, nullptr));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addDeviceCheck(const QSerialPortInfo &info) {
    qDebug()<<"addDeviceCheck"<<info.portName();

    QMutexLocker lock(&mWorkMutex);
    for(QList<QPair<WorkType, void*> >::iterator i = mWorkItems.begin(); i != mWorkItems.end(); i++) {
        if((*i).first != WorkTypeDeviceCheck) {
            continue;
        }

        if(static_cast<QSerialPortInfo*>((*i).second)->portName() == info.portName()) {
            return;
        }
    }

    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceCheck, new QSerialPortInfo(info)));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addDeviceRemove(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceRemove, device));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addDeviceClose(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceClose, device));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addDeviceProvision(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceProvision, device));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addDeviceProvisionTracking(Serial::SerialDevice* device, bool error, const QString& log) {
    qDebug()<<"addDeviceProvisionTracking";

    ProvisionTrackingItem* item = new ProvisionTrackingItem();
    item->device = device;
    item->error = error;
    item->log = log;

    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceProvisionTracking, item));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addCommand(SerialCommandItem *cmd) {
    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeCommand, cmd));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addLogin(const QString &username, const QString &password) {
    LoginItem* item = new LoginItem();
    item->username = username;
    item->password = password;
    item->token = "";

    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeLogin, item));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addLoginCheck(const QString& token) {
    LoginItem* item = new LoginItem();
    item->username = "";
    item->password = "";
    item->token = token;

    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeLogin, item));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::stop() {
    QMutexLocker stopLock(&mStoppedMutex);

    mRunning = false;

    // TODO: Make this wait until we are actually in the wait condition.
    mWaitCondition.wakeAll();

    // Wait for the worker to finish.
    QMutexLocker runningLock(&mRunningMutex);

    QMutexLocker lock(&mWorkMutex);
    for(QList<QPair<WorkType, void*> >::iterator i = mWorkItems.begin(); i != mWorkItems.end(); i++) {
        if((*i).first == WorkTypeDeviceCheck) {
            delete static_cast<QSerialPortInfo*>((*i).second);
        } else if((*i).first == WorkTypeCommand) {
            delete static_cast<SerialCommandItem*>((*i).second);
        } else if((*i).first == WorkTypeDeviceProvisionTracking) {
            delete static_cast<ProvisionTrackingItem*>((*i).second);
        } else if((*i).first == WorkTypeLogin) {
            delete static_cast<LoginItem*>((*i).second);
        }
    }

    mWorkItems.clear();
}

void UI::SerialDeviceWorker::process() {
    while(mRunning.load()) {
        bool waitForNextCommand = true;
        {
            QMutexLocker runningLock(&mRunningMutex);

            qDebug()<<"SerialDeviceWorker::process";

            // Process the devices list update first.
            int i = 0, size = 0;
            do {
                {
                    QMutexLocker locker(&mWorkMutex);
                    size = mWorkItems.size();
                    if(size <= 0 || i >= size || !mRunning.load()) {
                        break;
                    }

                    if(mWorkItems[i].first != WorkTypeDevicesListUpdate) {
                        i++;
                        continue;
                    }

                    mWorkItems.erase(mWorkItems.begin() + i);

                    size--;
                }

                processDevicesListUpdate();

                waitForNextCommand = false;
            } while(i < size);

            // We want to process all of the removals first as removing a device may also affect commands.
            i = size = 0;
            do {
                Serial::SerialDevice* device = nullptr;
                bool close = false;
                {
                    QMutexLocker locker(&mWorkMutex);
                    size = mWorkItems.size();
                    if(size <= 0 || i >= size || !mRunning.load()) {
                        break;
                    }

                    if(mWorkItems[i].first == WorkTypeDeviceRemove || mWorkItems[i].first == WorkTypeDeviceClose) {
                        device = static_cast<Serial::SerialDevice*>(mWorkItems[i].second);
                        close = mWorkItems[i].first == WorkTypeDeviceClose;

                        mWorkItems.erase(mWorkItems.begin() + i);

                        size--;
                    } else {
                        i++;
                        continue;
                    }
                }

                if(device != nullptr) {
                    processDeviceRemove(device, close);

                    waitForNextCommand = false;
                }
            } while(i < size);

            size = 0;
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
                case WorkTypeDeviceCheck: {
                    processDeviceCheck(static_cast<QSerialPortInfo*>(data));
                    break;
                }
                case WorkTypeDeviceProvision: {
                    processDeviceProvision(static_cast<Serial::SerialDevice*>(data));
                    break;
                }
                case WorkTypeDeviceProvisionTracking: {
                    processDeviceProvisionTracking(static_cast<ProvisionTrackingItem*>(data));
                    break;
                }
                case WorkTypeCommand: {
                    processCommand(static_cast<SerialCommandItem*>(data));
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

            qDebug()<<"Sleeping until next command";
            QMutexLocker locker(&mWakeMutex);
            mWaitCondition.wait(&mWakeMutex);
        }
    }

    // Wait for the stop handler to finish before returning
    QMutexLocker stopLock(&mStoppedMutex);
    qDebug()<<"SerialDeviceWorker finished";

    emit finished();
}

void UI::SerialDeviceWorker::processDeviceRemove(Serial::SerialDevice* device, bool close) {
    // We need to remove any pending commands/provisions that might be attached to a specific device.
    int i = 0, size = 0;
    do {
        QMutexLocker locker(&mWorkMutex);
        if(mWorkItems.size() == 0 || i >= mWorkItems.size()) {
            break;
        }

        if(mWorkItems[i].first == WorkTypeCommand) {
            SerialCommandItem* item = static_cast<SerialCommandItem*>(mWorkItems[i].second);
            if(item->device() == device) {
                item->deleteLater();
                mWorkItems.erase(mWorkItems.begin() + i);
            }
        } else if(mWorkItems[i].first == WorkTypeDeviceProvision) {
            Serial::SerialDevice* d = static_cast<Serial::SerialDevice*>(mWorkItems[i].second);
            if(d == device) {
                mWorkItems.erase(mWorkItems.begin() + i);
            }
        } else if(mWorkItems[i].first == WorkTypeDeviceProvision) {
           ProvisionTrackingItem* item = static_cast<ProvisionTrackingItem*>(mWorkItems[i].second);
           if(item->device == device) {
               delete item;
               mWorkItems.erase(mWorkItems.begin() + i);
           }
        } else {
            i++;
        }
    } while(i < size);

    if(!close) {
        delete device;
        device = nullptr;
    } else {
        device->close();
        emit deviceClose(device);
    }
}

void UI::SerialDeviceWorker::processDevicesListUpdate() {
    qDebug()<<"processDevicesListUpdate";

    emit statusChange("Updating devices list");

    QString token = QSettings().value("user/token").toString();

    QByteArray output;
    QHash<QString, QString> headers;
    headers.insert("U-Token", token);

    if(Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/devices.php"), output, headers)) {
        bool r = mDeviceConfig->update(QString(output));
        emit devicesListChanged(r);
    } else {
         emit devicesListChanged(false);
    }

    emit statusChange("");
}

void UI::SerialDeviceWorker::processDeviceCheck(QSerialPortInfo* portInfo) {
    emit statusChange("Getting device information for " + portInfo->portName());

    Serial::SerialDevice* device = new Serial::SerialDevice(*portInfo);
    if(!device->isValid()) {
        qWarning()<<"Device"<<portInfo->portName()<<"is not valid";
        delete device;
    } else {
        qDebug()<<"Device"<<portInfo->portName()<<"is valid";

        bool reschedule = false;
        if(device->update(&reschedule)) {
            if(!mDeviceConfig->updateDevice(device)) {
                qWarning()<<"Could not find device info for"<<portInfo->portName();
            }

            emit deviceAdd(device);
        } else if(reschedule) {
            emit deviceAddReschedule(device->port());
            delete device;
        }
    }

    emit statusChange("");

    delete portInfo;
}

void UI::SerialDeviceWorker::processDeviceProvision(Serial::SerialDevice* device) {
    emit statusChange("Provisioning " + device->name());

    QString userToken = QSettings().value("user/token").toString();

    connect(device, &Serial::SerialDevice::provisionProgressChanged, this, &UI::SerialDeviceWorker::deviceProvisionProgressChanged);
    bool ret = device->provision(userToken);
    disconnect(device, &Serial::SerialDevice::provisionProgressChanged, this, &UI::SerialDeviceWorker::deviceProvisionProgressChanged);
}

bool UI::SerialDeviceWorker::processDeviceProvisionTracking(ProvisionTrackingItem* item) {
    QString userToken = QSettings().value("user/token").toString();

    QByteArray output;
    QHash<QString, QString> headers;
    headers.insert("U-Token", userToken);

    QJsonObject deviceObj;
    deviceObj.insert("id", item->device->id());
    deviceObj.insert("min", item->device->newMinStr());
    deviceObj.insert("mdn", item->device->newMdnStr());
    deviceObj.insert("uniqueID", QString(QCryptographicHash::hash(item->device->imeiStr().toLatin1(), QCryptographicHash::Sha256).toHex()));

    QJsonObject obj;
    obj.insert("device", QJsonValue(deviceObj));

    if(item->error) {
        obj.insert("log", item->log);
    }

    QJsonDocument doc(obj);
    QString postData = doc.toJson();

    bool ret = Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/tracking.php"), output, headers, postData);

    delete item;
    return ret;
}

void UI::SerialDeviceWorker::processCommand(SerialCommandItem* item) {
    item->process();
    item->deleteLater();
}

void UI::SerialDeviceWorker::processLogin(LoginItem* item) {
    QByteArray output;
    QHash<QString, QString> headers;
    QString postData = "";

    if(item->token.size() > 0) {
        headers.insert("U-Token", item->token);
    } else {
        postData = "u=" + QString(QCryptographicHash::hash(item->username.toLatin1(), QCryptographicHash::Sha256).toHex()) + "&p=" + QString(QCryptographicHash::hash(item->password.toLatin1(), QCryptographicHash::Sha256).toHex());
    }

    if(!Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/login.php"), output, headers, postData)) {
        emit loginStatus(false, "");
    } else {
        if(item->token.size() > 0) {
            if(!processLoginCheckUpdate(item->token)) {
                emit loginStatus(true, item->token);
            }
        } else {
            QJsonDocument doc = QJsonDocument::fromJson(output);
            if(doc.isObject() && doc.object().contains("token")) {
                if(!processLoginCheckUpdate(doc.object()["token"].toString())) {
                    emit loginStatus(true, doc.object()["token"].toString());
                }
            } else {
                emit loginStatus(false, "");
            }
        }
    }

    delete item;
}

bool UI::SerialDeviceWorker::processLoginCheckUpdate(const QString& token) {
    emit updateCheck();

    QByteArray output;
    QHash<QString, QString> headers;
    headers.insert("U-Token", token);

    QString postData = "t=" + QString(PROG_BUILDTIME);

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

            emit updateAvailable(updaterDir.path(), token, obj["id"].toString(), obj["version"].toString(), QDateTime::fromTime_t(obj["time"].toInt()));

            return true;
        }
    }

    return false;
}

void UI::SerialDeviceWorker::deviceProvisionProgressChanged(int status, int progress, int error) {
    qDebug()<<"deviceProvisionProgressChanged"<<status<<progress<<error;

    emit provisionProgressChanged(static_cast<Serial::SerialDevice*>(sender()), status, progress, error);
}
