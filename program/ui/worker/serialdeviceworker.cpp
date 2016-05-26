#include <QDebug>
#include <QSettings>
#include <QThread>

#include "utils/fileutils.h"
#include "web/webutils.h"

#include "serialdeviceworker.h"

UI::Worker::SerialCommandItem::SerialCommandItem(Serial::SerialDevice* device)
    : mDevice(device)
{

}

UI::Worker::SerialCommandItem::~SerialCommandItem() {
    foreach(Serial::SerialCommand* cmd, mCmds) {
        delete cmd;
    }
}

void UI::Worker::SerialCommandItem::addItem(Serial::SerialCommand *cmd) {
    mCmds.append(cmd);
}

void UI::Worker::SerialCommandItem::process() {
    foreach(Serial::SerialCommand* cmd, mCmds) {
        cmd->execute();
    }

    emit finished();
}

UI::Worker::SerialDeviceWorker::SerialDeviceWorker()
    :   mThread(new QThread),
        mRunning(true),
        mDeviceConfig(nullptr)
{
    moveToThread(mThread);

    connect(mThread, &QThread::started, this, &SerialDeviceWorker::process);
    connect(this, &SerialDeviceWorker::finished, mThread, &QThread::quit);
    connect(this, &SerialDeviceWorker::finished, this, &QThread::deleteLater);
    connect(mThread, &QThread::finished, this, &SerialDeviceWorker::deleteLater);
}

UI::Worker::SerialDeviceWorker::~SerialDeviceWorker() {

}

void UI::Worker::SerialDeviceWorker::addDeviceCheck(const QSerialPortInfo &info) {
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

void UI::Worker::SerialDeviceWorker::addDeviceRemove(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceRemove, device));
    mWaitCondition.wakeAll();
}

void UI::Worker::SerialDeviceWorker::addDeviceClose(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceClose, device));
    mWaitCondition.wakeAll();
}

void UI::Worker::SerialDeviceWorker::addDeviceProvision(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeDeviceProvision, device));
    mWaitCondition.wakeAll();
}

void UI::Worker::SerialDeviceWorker::addCommand(SerialCommandItem *cmd) {
    QMutexLocker lock(&mWorkMutex);
    mWorkItems.append(QPair<WorkType, void*>(WorkTypeCommand, cmd));
    mWaitCondition.wakeAll();
}

void UI::Worker::SerialDeviceWorker::setDeviceConfig(Serial::SerialDeviceConfig* config) {
    mDeviceConfig = config;
}

void UI::Worker::SerialDeviceWorker::start() {
    mThread->start();
}

void UI::Worker::SerialDeviceWorker::stop() {
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
        }
    }

    mWorkItems.clear();
}

void UI::Worker::SerialDeviceWorker::process() {
    while(mRunning.load()) {
        bool waitForNextCommand = true;
        {
            QMutexLocker runningLock(&mRunningMutex);

            qDebug()<<"SerialDeviceWorker: process";

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
                case WorkTypeDeviceClose:
                case WorkTypeDeviceRemove: {
                    processDeviceRemove(static_cast<Serial::SerialDevice*>(data), type == WorkTypeDeviceClose);
                    break;
                }
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

            qDebug()<<"SerialDeviceWorker: Sleeping until next command";
            QMutexLocker locker(&mWakeMutex);
            mWaitCondition.wait(&mWakeMutex);
        }
    }

    // Wait for the stop handler to finish before returning
    QMutexLocker stopLock(&mStoppedMutex);
    qDebug()<<"SerialDeviceWorker: finished";

    emit finished();
}

void UI::Worker::SerialDeviceWorker::processDeviceRemove(Serial::SerialDevice* device, bool close) {
    if(device == nullptr) {
        return;
    }

    if(!close) {
        delete device;
        device = nullptr;
    } else {
        device->close();
        emit deviceClose(device);
    }
}

void UI::Worker::SerialDeviceWorker::processDeviceCheck(QSerialPortInfo* portInfo) {
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

void UI::Worker::SerialDeviceWorker::processDeviceProvision(Serial::SerialDevice* device) {
    if(device == nullptr) {
        return;
    }

    emit statusChange("Provisioning " + device->name());

    QString userToken = QSettings().value("user/token").toString();

    connect(device, &Serial::SerialDevice::provisionProgressChanged, this, &UI::Worker::SerialDeviceWorker::deviceProvisionProgressChanged);
    device->provision(userToken);
    disconnect(device, &Serial::SerialDevice::provisionProgressChanged, this, &UI::Worker::SerialDeviceWorker::deviceProvisionProgressChanged);
}

void UI::Worker::SerialDeviceWorker::processCommand(SerialCommandItem* item) {
    if(item->device() != nullptr) {
        item->process();
    }

    item->deleteLater();
}

void UI::Worker::SerialDeviceWorker::deviceProvisionProgressChanged(int status, int progress, int error) {
    qDebug()<<"deviceProvisionProgressChanged"<<status<<progress<<error;

    emit provisionProgressChanged(static_cast<Serial::SerialDevice*>(sender()), status, progress, error);
}
