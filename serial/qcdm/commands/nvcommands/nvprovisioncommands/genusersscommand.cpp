#include "genusersscommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

GenUserSSCommand::GenUserSSCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, QCDM::NV_DS_MIP_SS_USER_PROF_I));
    } else {
        if(jsonValue != nullptr) {
            QJsonObject jsonObj = jsonValue->toObject();

            uint8_t index = static_cast<uint8_t>(jsonObj["index"].toString().toUInt());
            QString mnHASharedSecret = jsonObj["mnHASharedSecret"].toString();
            QString mnAAASharedSecret = jsonObj["mnAAASharedSecret"].toString();

            QByteArray data;
            data.append(index);

            data.append(static_cast<uint8_t>(mnHASharedSecret.length()));
            QByteArray strData(16, 0x0);
            strData.replace(0, mnHASharedSecret.size(), mnHASharedSecret.toLatin1());

            data.append(strData);
            data.append(static_cast<uint8_t>(mnAAASharedSecret.size()));

            strData.fill(0x0, 16);
            strData.replace(0, mnAAASharedSecret.size(), mnAAASharedSecret.toLatin1());

            data.append(strData);

            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_DS_MIP_SS_USER_PROF_I, data));
        }
    }
}
