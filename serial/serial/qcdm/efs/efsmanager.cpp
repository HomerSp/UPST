#include <QDebug>
#include <fcntl.h>

#include "../../serialdevice.h"
#include "../commands/efscommand.h"
#include "efsmanager.h"

using namespace Serial::QCDM::EFS;

EFSManager::EFSManager(SerialDevice* device)
    : mDevice(device)
{

}

bool EFSManager::read(const QString& path, QByteArray& data) {
    int32_t fp = 0;
    if(!open(path, O_RDONLY, 0, fp)) {
        return false;
    }

    bool ret = false;

    EFSManager::FStat fileStat;
    if(fstat(fp, fileStat)) {
        ret = read(fp, fileStat.size, 0, data);
    }

    if(!close(fp)) {
        return false;
    }

    return ret;
}

bool EFSManager::write(const QString& path, const QByteArray &data) {
    int32_t fp = 0;
    if(!open(path, O_WRONLY | O_TRUNC, 0, fp)) {
        return false;
    }

    bool ret = write(fp, 0, data);
    if(!close(fp)) {
        return false;
    }

    return ret;
}

bool EFSManager::open(const QString& path, int32_t flags, int32_t createMode, int32_t& fp) {
    fp = 0;

    QByteArray frameData;
    frameData.append(static_cast<uint8_t>((flags) & 0xFF));
    frameData.append(static_cast<uint8_t>((flags >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((flags >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((flags >> 24) & 0xFF));
    frameData.append(static_cast<uint8_t>((createMode) & 0xFF));
    frameData.append(static_cast<uint8_t>((createMode >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((createMode >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((createMode >> 24) & 0xFF));
    frameData.append(path.toLatin1());
    frameData.append('\0');

    Serial::QCDM::Commands::EFSCommand cmd(mDevice, DIAG_EFS_OPEN, frameData);
    cmd.execute();
    if(!cmd.resultSuccess()) {
        return false;
    }

    QByteArray data = cmd.result()->data().toByteArray();
    if(data.size() < 8) {
        return false;
    }

    int32_t error = static_cast<int32_t>(data[4] & 0xFF)
            | (static_cast<int32_t>(data[5] & 0xFF) << 8)
            | (static_cast<int32_t>(data[6] & 0xFF) << 16)
            | (static_cast<int32_t>(data[7] & 0xFF) << 24);

    if(error != 0) {
        qCritical()<<"Open EFS file"<<path<<"failed,"<<error;
        return false;
    }

    fp = static_cast<int32_t>(data[0] & 0xFF)
        | (static_cast<int32_t>(data[1] & 0xFF) << 8)
        | (static_cast<int32_t>(data[2] & 0xFF) << 16)
        | (static_cast<int32_t>(data[3] & 0xFF) << 24);

    return true;
}

bool EFSManager::close(int32_t fp) {
    QByteArray frameData;
    frameData.append(static_cast<uint8_t>((fp) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 24) & 0xFF));

    Serial::QCDM::Commands::EFSCommand cmd(mDevice, DIAG_EFS_CLOSE, frameData);
    cmd.execute();
    if(!cmd.resultSuccess()) {
        return false;
    }

    QByteArray data = cmd.result()->data().toByteArray();
    if(data.size() < 4) {
        return false;
    }

    int32_t error = static_cast<int32_t>(data[0] & 0xFF)
            | (static_cast<int32_t>(data[1] & 0xFF) << 8)
            | (static_cast<int32_t>(data[2] & 0xFF) << 16)
            | (static_cast<int32_t>(data[3] & 0xFF) << 24);

    return error == 0;
}

bool EFSManager::fstat(int32_t fp, EFSManager::FStat& out) {
    QByteArray frameData;
    frameData.append(static_cast<uint8_t>((fp) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 24) & 0xFF));

    Serial::QCDM::Commands::EFSCommand cmd(mDevice, DIAG_EFS_FSTAT, frameData);
    cmd.execute();
    if(!cmd.resultSuccess()) {
        return false;
    }

    QByteArray data = cmd.result()->data().toByteArray();
    if(data.size() < 4 * 7) {
        return false;
    }

    int32_t error = static_cast<int32_t>(data[0] & 0xFF)
            | (static_cast<int32_t>(data[1] & 0xFF) << 8)
            | (static_cast<int32_t>(data[2] & 0xFF) << 16)
            | (static_cast<int32_t>(data[3] & 0xFF) << 24);

    if(error != 0) {
        qCritical()<<"FStat EFS file failed,"<<error;
        return false;
    }

    ::memcpy(&out, data.data() + 4, sizeof(EFSManager::FStat));
    return true;
}

bool EFSManager::read(int32_t fp, uint32_t size, uint32_t offset, QByteArray& output) {
    QByteArray frameData;
    frameData.append(static_cast<uint8_t>((fp) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 24) & 0xFF));
    frameData.append(static_cast<uint8_t>((size) & 0xFF));
    frameData.append(static_cast<uint8_t>((size >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((size >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((size >> 24) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset >> 24) & 0xFF));

    Serial::QCDM::Commands::EFSCommand cmd(mDevice, DIAG_EFS_READ, frameData);
    cmd.execute();
    if(!cmd.resultSuccess()) {
        return false;
    }

    QByteArray data = cmd.result()->data().toByteArray();
    if(data.size() < 4 * 4) {
        return false;
    }

    int32_t error = static_cast<int32_t>(data[12] & 0xFF)
            | (static_cast<int32_t>(data[13] & 0xFF) << 8)
            | (static_cast<int32_t>(data[14] & 0xFF) << 16)
            | (static_cast<int32_t>(data[15] & 0xFF) << 24);

    if(error != 0) {
        qCritical()<<"Reading EFS file failed,"<<error;
        return false;
    }

    int32_t bytesRead = static_cast<int32_t>(data[8] & 0xFF)
            | (static_cast<int32_t>(data[9] & 0xFF) << 8)
            | (static_cast<int32_t>(data[10] & 0xFF) << 16)
            | (static_cast<int32_t>(data[11] & 0xFF) << 24);

    if(data.size() < (4 * 4) + bytesRead) {
        qCritical()<<"Reading EFS file, wrong size,"<<data.size()<<"vs"<<((sizeof(int32_t) * 4) + bytesRead);
        return false;
    }

    for(int i = 16; i < 16 + bytesRead; i++) {
        output.append(data.at(i));
    }

    return true;
}

bool EFSManager::write(int32_t fp, uint32_t offset, const QByteArray& writeData) {
    QByteArray frameData;
    frameData.append(static_cast<uint8_t>((fp) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((fp >> 24) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset >> 8) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset >> 16) & 0xFF));
    frameData.append(static_cast<uint8_t>((offset >> 24) & 0xFF));
    frameData.append(writeData);

    Serial::QCDM::Commands::EFSCommand cmd(mDevice, DIAG_EFS_WRITE, frameData);
    cmd.execute();
    if(!cmd.resultSuccess()) {
        return false;
    }

    QByteArray data = cmd.result()->data().toByteArray();
    if(data.size() < 4 * 4) {
        return false;
    }

    int32_t error = static_cast<int32_t>(data[12] & 0xFF)
            | (static_cast<int32_t>(data[13] & 0xFF) << 8)
            | (static_cast<int32_t>(data[14] & 0xFF) << 16)
            | (static_cast<int32_t>(data[15] & 0xFF) << 24);

    if(error != 0) {
        qCritical()<<"Writing EFS file failed,"<<error;
        return false;
    }

    return true;
}
