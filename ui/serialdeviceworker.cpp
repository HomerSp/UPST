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

void UI::SerialDeviceWorker::addNewDevice(const QSerialPortInfo &info) {
    QMutexLocker lock(&mPortsMutex);
    foreach(const QSerialPortInfo *i, mPorts) {
        if(i->portName() == info.portName()) {
            return;
        }
    }

    mPorts.append(new QSerialPortInfo(info));
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::addCommand(SerialCommandItem *cmd) {
    QMutexLocker lock(&mCommandsMutex);

    mCommands.append(cmd);
    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::stop() {
    mRunning = false;

    // Wait for the worker to finish.
    QMutexLocker runningLock(&mRunningMutex);

    QMutexLocker lock(&mPortsMutex);
    foreach(const QSerialPortInfo *i, mPorts) {
        delete i;
    }

    mPorts.clear();

    QMutexLocker lock2(&mCommandsMutex);
    foreach(SerialCommandItem* item, mCommands) {
        delete item;
    }

    mCommands.clear();

    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::removeDevice(Serial::SerialDevice* device) {
    QMutexLocker locker(&mDeviceRemoveMutex);
    mDeviceRemove.append(device);

    mWaitCondition.wakeAll();
}

void UI::SerialDeviceWorker::process() {
    while(mRunning.load()) {
        bool hasNewDevices = false, hasDeviceRemove = false, hasCommands = false;
        {
            QMutexLocker runningLock(&mRunningMutex);

            qDebug()<<"SerialDeviceWorker::process";

            // We want to process all of the removals first as removing a device may also affect commands.
            {
                QMutexLocker locker(&mDeviceRemoveMutex);
                if(mDeviceRemove.size() > 0) {
                    hasDeviceRemove = true;
                }
            }

            if(hasDeviceRemove) {
                qDebug()<<"processDeviceRemovals";
                processDeviceRemovals();
            }

            {
                QMutexLocker locker(&mPortsMutex);
                if(mPorts.size() > 0) {
                    hasNewDevices = true;
                }
            }
            {
                QMutexLocker locker(&mCommandsMutex);
                if(mCommands.size() > 0) {
                    hasCommands = true;
                }
            }

            if(hasNewDevices) {
                qDebug()<<"processNewDevice";
                processNewDevice();
            }
            if(hasCommands) {
                qDebug()<<"processCommand";
                processCommand();
            }
        }

        // Wait until we have a new process item
        if(!hasNewDevices && !hasDeviceRemove && !hasCommands) {
            emit statusChange("");

            qDebug()<<"Sleeping until next command";
            QMutexLocker locker(&mWakeMutex);
            mWaitCondition.wait(&mWakeMutex);
        }
    }

    emit finished();
}

void UI::SerialDeviceWorker::processDeviceRemovals() {
    Serial::SerialDevice* device = nullptr;
    do {
        {
            QMutexLocker locker(&mDeviceRemoveMutex);

            if(mDeviceRemove.size() > 0) {
                device = *(mDeviceRemove.begin());
                mDeviceRemove.erase(mDeviceRemove.begin());
            } else {
                device = nullptr;
            }
        }

        if(device != nullptr) {
            QMutexLocker locker(&mCommandsMutex);
            for(int i = 0; i < mCommands.size(); i++) {
                SerialCommandItem* item = mCommands.at(i);
                if(item->device() == device) {
                    item->deleteLater();

                    mCommands.removeAt(i);
                    i--;
                }
            }

            delete device;
        }
    } while(device != nullptr);
}

void UI::SerialDeviceWorker::processNewDevice() {
    QSerialPortInfo *portInfo = nullptr;
    {
        QMutexLocker locker(&mPortsMutex);
        if(mPorts.size() > 0) {
            portInfo = *(mPorts.begin());
            mPorts.erase(mPorts.begin());
        } else {
            portInfo = nullptr;
        }
    }

    if(portInfo != nullptr) {
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
}

void UI::SerialDeviceWorker::processCommand() {
    SerialCommandItem* item = nullptr;
    {
        QMutexLocker locker(&mCommandsMutex);
        if(mCommands.size() > 0) {
            item = *(mCommands.begin());
            mCommands.erase(mCommands.begin());
        } else {
            item = nullptr;
        }
    }

    if(item != nullptr) {
        item->process();
        item->deleteLater();
    }
}
