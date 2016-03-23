#include <QDebug>

#include "prlcommand.h"

using namespace Serial::QCDM::Commands;

static uint32_t prlPacketSize = 120;

PRLCommand::PRLCommand(Serial::SerialDevice* device, bool read, QByteArray data)
    : QcdmCommand(device)
{
    if(!read) {
        // We need to round the frame count up to make sure all of the data fits.
        uint32_t frameCount = ceil(data.size() / (double)prlPacketSize);
        qDebug()<<"frameCount"<<frameCount;
        for(uint32_t i = 0; i < frameCount; i++) {
            QByteArray frameData;
            frameData.append(static_cast<uint8_t>(i));
            frameData.append(static_cast<uint8_t>((i == frameCount - 1)?0:1));
            frameData.append(static_cast<char>(0x0));   // NAM

            uint32_t frameSize = prlPacketSize;
            if(i == frameCount - 1) {
                frameSize = data.size() - (prlPacketSize * (frameCount - 1));
            }

            // Number of bits
            uint16_t bitFrameSize = frameSize * 8;
            frameData.append(static_cast<uint8_t>((bitFrameSize) & 0xFF));
            frameData.append(static_cast<uint8_t>((bitFrameSize >> 8) & 0xFF));
            frameData.append(data.mid(i * prlPacketSize, frameSize));
            for(uint16_t x = 0; x < 120 - frameSize; x++) {
                frameData.append(static_cast<char>(0x0));
            }

            qDebug()<<"addItem"<<frameSize;
            addItem(new QcdmCommandItem(QCDM::DIAG_PR_LIST_WR_F, frameData));
        }
    }
}

void PRLCommand::execute() {
    // We're writing data if we already have items at this point.
    if(count() > 0) {
        QcdmCommand::execute();
    // Otherwise we're going to read data.
    } else {
        QByteArray result;

        QByteArray writeData;
        writeData.append(static_cast<char>(0));
        writeData.append(static_cast<char>(0)); // NAM

        uint8_t i = 0;
        SerialCommandResult* res = nullptr;
        do {
            clearItems();

            writeData[0] = i;

            addItem(new QcdmCommandItem(QCDM::DIAG_PR_LIST_RD_F, writeData));
            QcdmCommand::execute();

            // TOAST approved code below.
            res = SerialCommand::results().last();
            if(res->success() && res->data().toByteArray().size() > 0) {
                QByteArray data = res->data().toByteArray();
                if(data.at(0) != 0x1) {
                    uint16_t size = ((static_cast<uint16_t>(data.at(6)) << 8) | (static_cast<uint8_t>(data.at(5)))) / 8;
                    result.append(data.mid(7, size));

                    // 0 means there is no more data.
                    if(data.at(4) == 0x00) {
                        res = nullptr;
                    }
                } else {
                    res = nullptr;
                }
            } else {
                res = nullptr;
            }
        } while(res != nullptr && ++i < 40);

        clearResults();

        addResult(result.size() > 0, result, 0);
    }
}
