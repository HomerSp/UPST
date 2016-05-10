#include "passwordcommand.h"

using namespace Serial::QCDM::Commands;

PasswordCommand::PasswordCommand(SerialDevice* device, QString password)
    : QcdmCommand(device)
{
    addItem(new QcdmCommandItem(Serial::QCDM::DIAG_PASSWORD_F, QByteArray::fromHex(password.toLatin1())));
}
