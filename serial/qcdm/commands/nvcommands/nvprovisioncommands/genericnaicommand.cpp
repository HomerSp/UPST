#include <QDebug>

#include "genericnaicommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

GenericNaiCommand::GenericNaiCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue, QCDM::NvItem item, NvItemType itemType, SerialProvisionData::UserType userType, uint8_t userProfIndex)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, item));
    } else {
        QString nai = jsonValue->toString();
        if(nai.startsWith("decesn@")) {
            nai = QString("%1").arg(device->esn(), 10, 10, QChar('0')) + nai.mid(nai.indexOf('@'));
        } else if(nai.startsWith("HSMDN@")) {
            nai = "HS" + device->mdnStr() + nai.mid(nai.indexOf('@'));
        } else if(nai.startsWith("MDN@")) {
            nai = device->mdnStr() + nai.mid(nai.indexOf('@'));
        } else if(nai.startsWith("MIN@")) {
            nai = device->minStr() + nai.mid(nai.indexOf('@'));
        } else if(!nai.contains("0")) {
            nai = SerialProvisionData::getUser(device, userType, nai);
        }

        QByteArray naiBytes(72, 0);
        naiBytes.replace(0, nai.size(), nai.toLatin1());

        QByteArray data;

        if(itemType == NV_GENERIC_MIP_NAI_BYTE_ARRAY) {
            data.append(userProfIndex);
        }

        data.append(static_cast<uint8_t>(nai.size()));
        data.append(naiBytes);

        addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, item, data));
    }
}
