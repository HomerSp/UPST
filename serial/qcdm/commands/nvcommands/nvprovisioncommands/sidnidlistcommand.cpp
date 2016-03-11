#include <QJsonArray>

#include "sidnidlistcommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

SidNidListCommand::SidNidListCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, QCDM::NV_HOME_SID_NID_I));
    } else {
        if(jsonValue != nullptr) {
            uint8_t nam = 0x0;

            QByteArray data;
            data.append(nam);

            QJsonArray arr = jsonValue->toArray();
            for(QJsonArray::const_iterator i = arr.constBegin(); i != arr.constEnd(); ++i) {
                QJsonObject obj = (*i).toObject();

                uint16_t sid = static_cast<uint16_t>(obj["sid"].toString().toUInt());
                data.append((sid) & 0xFF);
                data.append((sid >> 8) & 0xFF);

                uint16_t nid = static_cast<uint16_t>(obj["nid"].toString().toUInt());
                data.append((nid) & 0xFF);
                data.append((nid >> 8) & 0xFF);
            }

            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, NvItem::NV_HOME_SID_NID_I, data));
        }
    }
}
