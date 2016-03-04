#include <QDebug>

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

void UI::SerialDeviceWorker::addDeviceCheck(const QSerialPortInfo &info) {
    QMutexLocker lock(&mWorkMutex);
    for(QMap<WorkType, void*>::iterator i = mWorkItems.begin(); i != mWorkItems.end(); i++) {
        if(i.key() != WorkTypeDeviceCheck) {
            continue;
        }

        if(static_cast<QSerialPortInfo*>(i.value())->portName() == info.portName()) {
            return;
        }
    }

    mWorkItems.insert(WorkTypeDeviceCheck, new QSerialPortInfo(info));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addDeviceRemove(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);

    mWorkItems.insert(WorkTypeDeviceRemove, device);
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addDeviceProvision(Serial::SerialDevice* device) {
    QMutexLocker locker(&mWorkMutex);

    mWorkItems.insert(WorkTypeDeviceProvision, device);
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addCommand(SerialCommandItem *cmd) {
    QMutexLocker lock(&mWorkMutex);

    mWorkItems.insert(WorkTypeCommand, cmd);
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::stop() {
    mRunning = false;

    // Wait for the worker to finish.
    QMutexLocker runningLock(&mRunningMutex);

    QMutexLocker lock(&mWorkMutex);
    for(QMap<WorkType, void*>::iterator i = mWorkItems.begin(); i != mWorkItems.end(); i++) {
        if(i.key() == WorkTypeDeviceCheck) {
            delete static_cast<QSerialPortInfo*>(i.value());
        } else if(i.key() == WorkTypeCommand) {
            delete static_cast<SerialCommandItem*>(i.value());
        }
    }

    mWorkItems.clear();

    mWaitCondition.wakeAll();
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
                    if(size == 0) {
                        break;
                    }

                    if(mWorkItems.keys()[i++] != WorkTypeDeviceRemove) {
                        continue;
                    }

                    device = static_cast<Serial::SerialDevice*>(mWorkItems.values()[i]);
                    mWorkItems.erase(mWorkItems.begin() + i);

                    i--;
                    size--;
                }

                processDeviceRemove(device);

                waitForNextCommand = false;
            } while(i < size);

            size = 0;
            do {
                WorkType type;
                void* data = nullptr;

                {
                    QMutexLocker locker(&mWorkMutex);
                    size = mWorkItems.size();
                    if(size == 0) {
                        break;
                    }

                    type = mWorkItems.begin().key();
                    data = mWorkItems.begin().value();
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
                default:
                    break;
                }

                waitForNextCommand = false;
            } while(size > 0);
        }

        // Wait until we have a new process item
        if(waitForNextCommand) {
            emit statusChange("");

            qDebug()<<"Sleeping until next command";
            QMutexLocker locker(&mWakeMutex);
            mWaitCondition.wait(&mWakeMutex);
        }
    }

    emit finished();
}

void UI::SerialDeviceWorker::processDeviceRemove(Serial::SerialDevice* device) {
    int i = 0, size = 0;
    do {
        QMutexLocker locker(&mWorkMutex);
        size = mWorkItems.size();
        if(mWorkItems.keys()[i++] != WorkTypeCommand) {
            continue;
        }

        SerialCommandItem* item = static_cast<SerialCommandItem*>(mWorkItems.values()[i]);
        if(item->device() == device) {
            item->deleteLater();
            mWorkItems.erase(mWorkItems.begin() + i);
            i--;
            size--;
        }
    } while(i < size);

    delete device;
}

void UI::SerialDeviceWorker::processDeviceCheck(QSerialPortInfo* portInfo) {
    emit statusChange("Getting device information for " + portInfo->portName());

    Serial::SerialDevice* device = new Serial::SerialDevice(*portInfo);
    if(!device->isValid()) {
        qWarning()<<"Device is not valid";
        delete device;
    } else {
        qDebug()<<"Device is valid";
        device->update();
        mDeviceConfig->updateDevice(device);
        emit deviceAdd(device);
    }

    emit statusChange("");

    delete portInfo;
}

void UI::SerialDeviceWorker::processDeviceProvision(Serial::SerialDevice* device) {
    emit statusChange("Provisioning " + device->name());
    device->provision();
}

void UI::SerialDeviceWorker::processCommand(SerialCommandItem* item) {
    item->process();
    item->deleteLater();
}
