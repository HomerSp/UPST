#include <QHostAddress>

#include "genuserprofcommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

GenUserProfCommand::GenUserProfCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue, QString userNai)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, QCDM::NV_DS_MIP_GEN_USER_PROF_I));
    } else {
        if(jsonValue != nullptr) {
            QJsonObject jsonObj = jsonValue->toObject();

            uint8_t index = static_cast<uint8_t>(jsonObj["index"].toString().toUInt());
            uint8_t naiLength = userNai.size();
            QByteArray naiBytes(72, 0);
            naiBytes.replace(0, userNai.size(), userNai.toLatin1());

            uint8_t mnAAASpiSet = static_cast<uint8_t>(jsonObj["mnAAASpiSet"].toString().toUInt());
            uint32_t mnAAASpi = jsonObj["mnAAASpi"].toString().toUInt();
            uint8_t mnHASpiSet = static_cast<uint8_t>(jsonObj["mnHASpiSet"].toString().toUInt());
            uint32_t mnHASpi = jsonObj["mnHASpi"].toString().toUInt();
            uint8_t retTunPref = static_cast<uint8_t>(jsonObj["retTunPref"].toString().toUInt());
            uint32_t homeAddress = QHostAddress(jsonObj["homeAddress"].toString()).toIPv4Address();
            uint32_t primaryAddress = QHostAddress(jsonObj["primaryAddress"].toString()).toIPv4Address();
            uint32_t secondaryAddress = QHostAddress(jsonObj["secondaryAddress"].toString()).toIPv4Address();

            QByteArray data;
            data.append(index);
            data.append(naiLength);
            data.append(naiBytes);
            data.append(mnAAASpiSet);
            data.append((mnAAASpi) & 0xFF);
            data.append((mnAAASpi >> 8) & 0xFF);
            data.append((mnAAASpi >> 16) & 0xFF);
            data.append((mnAAASpi >> 24) & 0xFF);
            data.append(mnHASpiSet);
            data.append((mnHASpi) & 0xFF);
            data.append((mnHASpi >> 8) & 0xFF);
            data.append((mnHASpi >> 16) & 0xFF);
            data.append((mnHASpi >> 24) & 0xFF);
            data.append(retTunPref);
            data.append((homeAddress) & 0xFF);
            data.append((homeAddress >> 8) & 0xFF);
            data.append((homeAddress >> 16) & 0xFF);
            data.append((homeAddress >> 24) & 0xFF);
            data.append((primaryAddress) & 0xFF);
            data.append((primaryAddress >> 8) & 0xFF);
            data.append((primaryAddress >> 16) & 0xFF);
            data.append((primaryAddress >> 24) & 0xFF);
            data.append((secondaryAddress) & 0xFF);
            data.append((secondaryAddress >> 8) & 0xFF);
            data.append((secondaryAddress >> 16) & 0xFF);
            data.append((secondaryAddress >> 24) & 0xFF);

            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, QCDM::NV_DS_MIP_GEN_USER_PROF_I, data));
        }
    }
}
