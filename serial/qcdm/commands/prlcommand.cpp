#include "prlcommand.h"

using namespace Serial::QCDM::Commands;

PRLCommand::PRLCommand(Serial::SerialDevice* device, bool read, QByteArray data)
    : QcdmCommand(device)
{
    uint32_t prlPacketSize = 120;

    if(!read) {
        uint32_t frameCount = ceil(data.size() / (double)prlPacketSize);
        for(uint32_t i = 0; i < frameCount; i++) {
            QByteArray frameData;
            frameData.append(static_cast<uint8_t>(i));
            frameData.append(static_cast<uint8_t>((i == frameCount - 1)?0:1));
            frameData.append(static_cast<char>(0x0));

            uint32_t frameSize = prlPacketSize;
            if(i == frameCount - 1) {
                frameSize = data.size() - (prlPacketSize * (frameCount - 1));
            }

            frameData.append(static_cast<uint8_t>(frameSize << 3 & 0xFF));
            frameData.append(static_cast<uint8_t>(frameSize >> 5 & 0xFF));
            frameData.append(data.mid(i * prlPacketSize, frameSize));

            addItem(new QcdmCommandItem(QCDM::DIAG_PR_LIST_WR_F, frameData));
        }
    }
}

