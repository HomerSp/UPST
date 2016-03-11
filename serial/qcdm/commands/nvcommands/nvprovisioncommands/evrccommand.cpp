#include "evrccommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

EvrcCommand::EvrcCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, NvItem::NV_PREF_VOICE_SO_I));
    } else {
        if(jsonValue != nullptr) {
            QJsonObject jsonObj = jsonValue->toObject();

            uint8_t nam = 0x0;
            uint8_t enabled = static_cast<uint8_t>(jsonObj["_enabled"].toString().toUInt());
            uint16_t homePageSo = static_cast<uint16_t>(jsonObj["home_page_so"].toString().toUInt());
            uint16_t homeOrigSo = static_cast<uint16_t>(jsonObj["home_orig_so"].toString().toUInt());
            uint16_t roamOrigSo = static_cast<uint16_t>(jsonObj["roam_orig_so"].toString().toUInt());

            QByteArray data;
            data.append(nam);
            data.append(enabled);
            data.append((homePageSo) & 0xFF);
            data.append((homePageSo >> 8) & 0xFF);
            data.append((homeOrigSo) & 0xFF);
            data.append((homeOrigSo >> 8) & 0xFF);
            data.append((roamOrigSo) & 0xFF);
            data.append((roamOrigSo >> 8) & 0xFF);

            addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, NvItem::NV_PREF_VOICE_SO_I, data));
        }
    }
}
