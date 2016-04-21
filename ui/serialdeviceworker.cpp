#include <QDebug>
#include <QThread>

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
    : mRunning(true)
{
    mDeviceConfig = new Serial::SerialDeviceConfig();
}

UI::SerialDeviceWorker::~SerialDeviceWorker() {
    delete mDeviceConfig;
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

void UI::SerialDeviceWorker::addDeviceProvision(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);

    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceProvision, device));
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

            // We want to process all of the removals first as removing a device may also affect commands.
            int i = 0, size = 0;
            do {
                Serial::SerialDevice* device = nullptr;
                {
                    QMutexLocker locker(&mWorkMutex);
                    size = mWorkItems.size();
                    if(size <= 0 || i >= size || !mRunning.load()) {
                        break;
                    }

                    if(mWorkItems[i].first != WorkTypeDeviceRemove) {
                        i++;
                        continue;
                    }

                    device = static_cast<Serial::SerialDevice*>(mWorkItems[i].second);
                    mWorkItems.erase(mWorkItems.begin() + i);

                    size--;
                }

                if(device != nullptr) {
                    processDeviceRemove(device);

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
                    mWorkItems.erase(mWorkItems.begin());
                }

                switch(type) {
                case WorkTypeDeviceCheck: {
                    processDeviceCheck(static_cast<QSerialPortInfo*>(data));
                    break;
                }
                case WorkTypeDeviceProvision: {
                    processDeviceProvision(static_cast<Serial::SerialDevice*>(data));
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
                    break;
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

void UI::SerialDeviceWorker::processDeviceRemove(Serial::SerialDevice* device) {
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
        } else {
            i++;
        }
    } while(i < size);

    delete device;
    device = nullptr;
}

void UI::SerialDeviceWorker::processDeviceCheck(QSerialPortInfo* portInfo) {
    emit statusChange("Getting device information for " + portInfo->portName());

    Serial::SerialDevice* device = new Serial::SerialDevice(*portInfo);
    if(!device->isValid()) {
        qWarning()<<"Device"<<portInfo->portName()<<"is not valid";
        delete device;
    } else {
        qDebug()<<"Device"<<portInfo->portName()<<"is valid";
        device->update();
#ifndef TESTING_MODE
        if(mDeviceConfig->updateDevice(device)) {
            emit deviceAdd(device);
        }
#else
        mDeviceConfig->updateDevice(device);
        emit deviceAdd(device);
#endif
    }

    emit statusChange("");

    delete portInfo;
}

void UI::SerialDeviceWorker::processDeviceProvision(Serial::SerialDevice* device) {
    emit statusChange("Provisioning " + device->name());

    connect(device, &Serial::SerialDevice::provisionProgressChanged, this, &UI::SerialDeviceWorker::deviceProvisionProgressChanged);
    device->provision();
    disconnect(device, &Serial::SerialDevice::provisionProgressChanged, this, &UI::SerialDeviceWorker::deviceProvisionProgressChanged);
}

void UI::SerialDeviceWorker::processCommand(SerialCommandItem* item) {
    item->process();
    item->deleteLater();
}

void UI::SerialDeviceWorker::processLogin(LoginItem* item) {
    emit loginStatus(true);

    delete item;
}

void UI::SerialDeviceWorker::deviceProvisionProgressChanged(Serial::SerialProvisionStatus status, int progress, Serial::SerialProvisionError error) {
    qDebug()<<"deviceProvisionProgressChanged"<<status<<progress<<error;

    emit provisionProgressChanged(static_cast<Serial::SerialDevice*>(sender()), status, progress, error);
}
