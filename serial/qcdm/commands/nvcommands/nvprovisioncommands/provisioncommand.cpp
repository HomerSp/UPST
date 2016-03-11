#include <QHostAddress>

#include "provisioncommand.h"

using namespace Serial::QCDM::Commands::Nv::Provision;

ProvisionCommand::ProvisionCommand(SerialDevice* device, bool read, const QJsonValue* jsonValue, QCDM::NvItem item, NvItemType itemType, uint8_t userProfIndex)
    : NvCommand(device)
{
    if(read) {
        addItem(new NvCommandItem(QCDM::DIAG_NV_READ_F, item));
    } else {
        QByteArray data;

        switch(itemType) {
        case NV_GENERIC_BYTE_ARRAY: {
            data.append(static_cast<uint8_t>(jsonValue->toString().toUInt()));
            break;
        }
        case NV_GENERIC_FLAG_ARRAY: {
            data.append(static_cast<uint8_t>(jsonValue->toString().toUInt() != 0));
            break;
        }
        case NV_GENERIC_NAM_BYTE_ARRAY: {
            data.append((char)0x0);
            data.append(static_cast<uint8_t>(jsonValue->toString().toUInt()));
            break;
        }
        case NV_GENERIC_NAM_FLAG_ARRAY: {
            data.append((char)0x0);
            data.append(static_cast<uint8_t>(jsonValue->toString().toUInt() != 0));
            break;
        }
        case NV_GENERIC_MIP_BYTE_ARRAY: {
            data.append(userProfIndex);
            data.append(static_cast<uint8_t>(jsonValue->toString().toUInt()));
            break;
        }
        case NV_GENERIC_IP_BYTE_ARRAY: {
            uint32_t addr = QHostAddress(jsonValue->toString()).toIPv4Address();
            data.append((addr) & 0xFF);
            data.append((addr >> 8) & 0xFF);
            data.append((addr >> 16) & 0xFF);
            data.append((addr >> 24) & 0xFF);

            break;
        }
        case NV_GENERIC_NAM_UINT16_ARRAY: {
            uint16_t val = static_cast<uint16_t>(jsonValue->toString().toUInt());
            data.append((char)0x0);
            data.append((val) & 0xFF);
            data.append((val >> 8) & 0xFF);
            break;
        }
        case NV_GENERIC_NAM_ANSI_ARRAY: {
            QByteArray bytes(96, 0);
            bytes.replace(0, jsonValue->toString().size(), jsonValue->toString().toLatin1());

            data.append((char)0x0);
            data.append(bytes);

            break;
        }
        default:
            break;
        }

        addItem(new NvCommandItem(QCDM::DIAG_NV_WRITE_F, item, data));
    }
}

