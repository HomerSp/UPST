#include <QDebug>
#include "serial/qcdm/commands/qcdmcommand.h"

#include "manual.h"

UI::Section::Manual::Manual(UI::MainUI* ui)
    : UISection(ui)
{
    QObject* sendButton = rootObject()->findChild<QObject*>("manualRawSend");
    QObject::connect(sendButton, SIGNAL(clicked()), this, SLOT(rawSend()));
}

UI::Section::Manual::~Manual() {

}

void UI::Section::Manual::update() {
    UISection::startUpdate();

    UISection::endUpdate();
}

void UI::Section::Manual::rawSend() {
    QObject* inputText = rootObject()->findChild<QObject*>("manualRawInput");

    QByteArray inputData = QByteArray::fromHex(inputText->property("text").toString().toLatin1());
    if(inputData.size() <= 0) {
        return;
    }

    qDebug()<<"rawSend"<<QString(inputData.toHex());

    Serial::SerialDevice* currentDevice = UISection::currentDevice();
    if(currentDevice != nullptr) {
        UI::Worker::SerialCommandItem* cmdItem = new UI::Worker::SerialCommandItem(currentDevice);
        connect(cmdItem, &UI::Worker::SerialCommandItem::finished, this, &UI::Section::Manual::rawCommandFinished);

        cmdItem->addItem(new Serial::QCDM::Commands::QcdmCommand(currentDevice, (Serial::QCDM::DiagCommands)inputData.at(0), inputData.mid(1)));

        ui()->deviceWorker()->addCommand(cmdItem);
    }
}

void UI::Section::Manual::rawCommandFinished() {
    QObject* outputText = rootObject()->findChild<QObject*>("manualRawOutput");

    UI::Worker::SerialCommandItem* item = static_cast<UI::Worker::SerialCommandItem*>(sender());
    Serial::QCDM::Commands::QcdmCommand *cmd = static_cast<Serial::QCDM::Commands::QcdmCommand*>(item->cmds().at(0));
    outputText->setProperty("text", QString(cmd->result()->data().toByteArray().toHex()));
}
