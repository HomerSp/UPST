#include <QObject>
#include <QDebug>

#include "serialcommunicator.h"
#include "serialdevice.h"
#include "qcdm/commands/nv/esncommand.h"
#include "qcdm/commands/nv/imeicommand.h"
#include "qcdm/commands/nv/meidcommand.h"
#include "qcdm/commands/nv/mdncommand.h"

using namespace Serial;

SerialDevice::SerialDevice(const QString& port, uint16_t vid, uint16_t pid, const QString& manufacturer, const QString& description, bool isAndroid)
    : mCommunicator(nullptr),
      mPort(port), mVid(vid), mPid(pid),
      mManufacturer(manufacturer),
      mDescription(description),
      mIsAndroid(isAndroid),
      mBaudRate(QSerialPort::Baud115200),
      mDataBits(QSerialPort::Data8),
      mParity(QSerialPort::NoParity),
      mStopBits(QSerialPort::OneStop)
{
    mCommunicator = new SerialCommunicator(*this);
    mCommunicator->open();
    mCommunicator->clear();
}

SerialDevice::SerialDevice(const QSerialPortInfo& info, bool isAndroid)
    : SerialDevice(info.portName(), info.vendorIdentifier(), info.productIdentifier(), info.manufacturer(), info.description(), isAndroid) {

}

SerialDevice::~SerialDevice() {
    if(mCommunicator != nullptr) {
        delete mCommunicator;
    }

    mCommunicator = nullptr;
}

SerialCommunicator* SerialDevice::communicator() {
    return mCommunicator;
}

bool SerialDevice::isValid() {
    Serial::QCDM::Commands::QcdmCommand cmd(communicator(), Serial::QCDM::DiagCommands::DIAG_VERNO_F);
    cmd.setTimeout(1000);

    QByteArray result;
    return cmd.execute(result);
}

bool SerialDevice::update() {
    uint32_t esn;
    uint64_t imei, meid;

    qDebug()<<"===== GETTING ESN =====";
    Serial::QCDM::Commands::Nv::ESNCommand esnCmd(communicator(), Serial::QCDM::DiagCommands::DIAG_NV_READ_F);
    if(!esnCmd.execute(esn)) {
        return false;
    }

    qDebug()<<"===== GETTING IMEI =====";
    Serial::QCDM::Commands::Nv::IMEICommand imeiCmd(communicator(), Serial::QCDM::DiagCommands::DIAG_NV_READ_F);
    if(!imeiCmd.execute(imei)) {
        return false;
    }

    qDebug()<<"===== GETTING MEID =====";
    Serial::QCDM::Commands::Nv::MEIDCommand meidCmd(communicator(), Serial::QCDM::DiagCommands::DIAG_NV_READ_F);
    if(!meidCmd.execute(meid)) {
        return false;
    }

    qDebug()<<"===== GETTING MDN =====";
    Serial::QCDM::Commands::Nv::MDNCommand mdnCmd(communicator(), Serial::QCDM::DiagCommands::DIAG_NV_READ_F);
    if(!mdnCmd.execute(mMdn)) {
        return false;
    }

    qDebug()<<"ESN:"<<QString::number(esn, 16)<<"IMEI:"<<QString::number(imei, 16)<<"MEID:"<<QString::number(meid, 16)<<"MDN:"<<mMdn;

    return true;
}

bool SerialDevice::operator==(const SerialDevice& other) {
    return other.mPort == mPort;
}

bool SerialDevice::operator==(const QString& port) {
    return port == mPort;
}
