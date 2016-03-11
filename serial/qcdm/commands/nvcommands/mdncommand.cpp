#include <QString>

#include "mdncommand.h"

using namespace Serial::QCDM::Commands::Nv;

MDNCommand::MDNCommand(SerialDevice* device, bool read, QString data)
    : NvCommandString(device, read, NvItem::NV_DIR_NUMBER_I)
{
    QByteArray mdnData = data.toLatin1();
    mdnData.insert(0, (QChar)0x0);
    setData(mdnData, 0);
}
