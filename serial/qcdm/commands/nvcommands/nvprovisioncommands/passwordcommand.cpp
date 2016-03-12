#include "../../../../serialprovisiondata.h"
#include "passwordcommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

PasswordCommand::PasswordCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue)
    : NvCommand(device)
{
    if(!read) {
        if(jsonValue != nullptr) {
            QString password = SerialProvisionData::getPassword(device, jsonValue->toString());

            QByteArray strData;
            strData.fill(0x0, 72);
            strData.replace(0, password.size(), password.toLatin1());

            QByteArray data;
            data.append(static_cast<uint8_t>(password.size()));
            data.append(strData);

            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_PPP_PASSWORD_I, data));
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_PAP_PASSWORD_I, data));
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_HDR_AN_AUTH_PASSWORD_I, data));
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_HDR_AN_AUTH_PASSWD_LONG_I, data));
            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_HDR_AN_PPP_PASSWORD_I, data));
        }
    }
}
