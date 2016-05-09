#include "hdranusercommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

HDRAnUserCommand::HDRAnUserCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue, NvItemType itemType, SerialProvisionData::UserType userType, uint8_t userProfIndex)
    : GenericNaiCommand(device, read, jsonValue, NvItem::NV_HDR_AN_AUTH_NAI_I, itemType, userType, userProfIndex)
{
    if(!read) {
        NvCommandItem* naiItem = static_cast<NvCommandItem*>(item(0));
        NvCommandItem* naiLongItem = new NvCommandItem(naiItem);
        naiLongItem->nvItem = NvItem::NV_HDR_AN_AUTH_USER_ID_LONG_I;
        addItem(naiLongItem);
    }
}

