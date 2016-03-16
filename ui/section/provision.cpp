#include <QDebug>

#include "../../serial/qcdm/commands/qcdmcommand.h"

#include "provision.h"

UI::Section::Provision::Provision(UI::MainUI* ui)
    : UISection(ui)
{
    QObject* provisionButton = rootObject()->findChild<QObject*>("provisionButton");
    QObject::connect(provisionButton, SIGNAL(clicked()), this, SLOT(provision()));
}

UI::Section::Provision::~Provision() {

}

void UI::Section::Provision::update() {
    UISection::startUpdate();

    UISection::endUpdate();
}

void UI::Section::Provision::provision() {
    QObject* rootObject = UISection::rootObject();
    QString mdn = rootObject->findChild<QObject*>("textMDN")->property("text").toString();
    uint64_t min = (uint64_t)rootObject->findChild<QObject*>("textMIN")->property("text").toString().toULongLong();

    Serial::SerialDevice* device = currentDevice();
    if(device == nullptr) {
        return;
    }

    qDebug()<<"Provision"<<device->name()<<mdn<<min;

    device->setProvisionData(mdn, min);

    ui()->worker()->addDeviceProvision(device);

}
