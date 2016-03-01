#include <QDebug>
#include "../../serial/qcdm/commands/nv/nvcommand.h"

#include "manual.h"

UI::Section::Manual::Manual(UI::MainUI* ui)
    : UISection(ui)
{

}

UI::Section::Manual::~Manual() {

}

void UI::Section::Manual::update() {
    UISection::startUpdate();

    Serial::SerialDevice* currentDevice = UISection::currentDevice();
    if(currentDevice != nullptr) {
        SerialCommandItem* cmdItem = new SerialCommandItem(currentDevice);
        connect(cmdItem, &SerialCommandItem::finished, this, &UI::Section::Manual::manualCommandFinished);

        cmdItem->addItem(new Serial::QCDM::Commands::Nv::NvCommand16Bit(currentDevice, true, Serial::QCDM::NV_MOB_FIRM_REV_I));
        cmdItem->addItem(new Serial::QCDM::Commands::Nv::NvCommand16Bit(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_MOB_MODEL_I));
        cmdItem->addItem(new Serial::QCDM::Commands::Nv::NvCommand32Bit(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_MOB_CAI_REV_I));
        cmdItem->addItem(new Serial::QCDM::Commands::Nv::NvCommandString(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_SW_VERSION_INFO_I));
        if(currentDevice->vid() == 0x04e8) {
            cmdItem->addItem(new Serial::QCDM::Commands::Nv::NvCommandString(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_OEM_SAMSUNG_MODEL));
        }

        ui()->worker()->addCommand(cmdItem);
    }

    UISection::endUpdate();
}

void UI::Section::Manual::manualCommandFinished() {
    SerialCommandItem* item = static_cast<SerialCommandItem*>(sender());
    QString data = "VID = " + item->device()->vidStr() + "\nPID = " + item->device()->pidStr();
    {
        Serial::QCDM::Commands::Nv::NvCommand16Bit *cmd = static_cast<Serial::QCDM::Commands::Nv::NvCommand16Bit*>(item->cmds().at(0));
        if(cmd->resultSuccess()) {
            data += QString("\n") + "NV_MOB_FIRM_REV_I = 0x" + QString::number(cmd->resultData(), 16);
        }
    }
    {
        Serial::QCDM::Commands::Nv::NvCommand16Bit *cmd = static_cast<Serial::QCDM::Commands::Nv::NvCommand16Bit*>(item->cmds().at(1));
        if(cmd->resultSuccess()) {
            data += QString("\n") + "NV_MOB_MODEL_I = 0x" + QString::number(cmd->resultData(), 16);
        }
    }
    {
        Serial::QCDM::Commands::Nv::NvCommand32Bit *cmd = static_cast<Serial::QCDM::Commands::Nv::NvCommand32Bit*>(item->cmds().at(2));
        if(cmd->resultSuccess()) {
            data += QString("\n") + "NV_MOB_CAI_REV_I = 0x" + QString::number(cmd->resultData(), 16);
        }
    }
    {
        Serial::QCDM::Commands::Nv::NvCommandString *cmd = static_cast<Serial::QCDM::Commands::Nv::NvCommandString*>(item->cmds().at(3));
        if(cmd->resultSuccess()) {
            data += QString("\n") + "NV_SW_VERSION_INFO_I = " + cmd->resultData();
        }
    }
    if(item->device()->vid() == 0x04e8) {
        Serial::QCDM::Commands::Nv::NvCommandString *cmd = static_cast<Serial::QCDM::Commands::Nv::NvCommandString*>(item->cmds().at(4));
        if(cmd->resultSuccess()) {
            data += QString("\n") + "NV_OEM_SAMSUNG_MODEL = " + cmd->resultData();
        }
    }

    QObject* manualModeOutput = rootObject()->findChild<QObject*>("manualModeOutput");
    manualModeOutput->setProperty("text", data);

}
