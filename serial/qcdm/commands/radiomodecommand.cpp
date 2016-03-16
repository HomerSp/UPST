#include "radiomodecommand.h"

using namespace Serial::QCDM::Commands;

RadioModeCommand::RadioModeCommand(SerialDevice* device, Serial::QCDM::Mode mode)
    : QcdmCommand(device)
{
    QByteArray resetArray;
    resetArray.append(static_cast<char>(mode));
    resetArray.append(static_cast<char>(0x0));

    addItem(new QcdmCommandItem(Serial::QCDM::DIAG_CONTROL_F, resetArray));
}

