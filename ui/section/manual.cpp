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
        QString data = "VID = " + currentDevice->vidStr() + "\nPID = " + currentDevice->pidStr();
        {
            Serial::QCDM::Commands::Nv::NvCommand16Bit swRevCmd(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_MOB_FIRM_REV_I);

            uint16_t output = 0;
            if(!swRevCmd.execute(output)) {
                qDebug()<<"Could not get data";
            }

            data += QString("\n") + "NV_MOB_FIRM_REV_I = 0x" + QString::number(output, 16);
        }
        {
            Serial::QCDM::Commands::Nv::NvCommand16Bit swRevCmd(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_MOB_MODEL_I);

            uint16_t output = 0;
            if(!swRevCmd.execute(output)) {
                qDebug()<<"Could not get data";
            }

            data += QString("\n") + "NV_MOB_MODEL_I = 0x" + QString::number(output, 16);
        }
        {
            Serial::QCDM::Commands::Nv::NvCommand32Bit swRevCmd(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_MOB_CAI_REV_I);

            uint32_t output = 0;
            if(!swRevCmd.execute(output)) {
                qDebug()<<"Could not get data";
            }

            data += QString("\n") + "NV_MOB_CAI_REV_I = 0x" + QString::number(output, 16);
        }
        {
            Serial::QCDM::Commands::Nv::NvCommandString swRevCmd(currentDevice, Serial::QCDM::DIAG_NV_READ_F, Serial::QCDM::NV_SW_VERSION_INFO_I);

            QString output = 0;
            if(!swRevCmd.execute(output)) {
                qDebug()<<"Could not get data";
            }

            data += QString("\n") + "NV_SW_VERSION_INFO_I = " + output;
        }

        QObject* manualModeOutput = rootObject()->findChild<QObject*>("manualModeOutput");
        manualModeOutput->setProperty("text", data);
    }

    UISection::endUpdate();
}
