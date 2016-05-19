#include <QDebug>

#include "serial/qcdm/commands/qcdmcommand.h"

#include "provision.h"

UI::Section::Provision::Provision(UI::MainUI* ui)
    : UISection(ui)
{
    QObject* provisionButton = rootObject()->findChild<QObject*>("provisionButton");
    QObject::connect(provisionButton, SIGNAL(clicked()), this, SLOT(provision()));
}

UI::Section::Provision::~Provision() {
    beforeDeviceChanged();
}

void UI::Section::Provision::beforeDeviceChanged() {
    if(currentDevice() == nullptr) {
        return;
    }

    QObject* rootObject = UISection::rootObject();
    QString mdn = rootObject->findChild<QObject*>("textMDN")->property("text").toString();
    uint64_t min = (uint64_t)rootObject->findChild<QObject*>("textMIN")->property("text").toString().toULongLong();

    currentDevice()->setProvisionData(mdn, min);
}

void UI::Section::Provision::update() {
    UISection::startUpdate();

    Serial::SerialDevice* device = currentDevice();
    if(device != nullptr) {
        QObject* rootObject = UISection::rootObject();
        rootObject->findChild<QObject*>("textMDN")->setProperty("text", device->newMdnStr());
        rootObject->findChild<QObject*>("textMIN")->setProperty("text", device->newMinStr());

        rootObject->findChild<QObject*>("textMDN")->setProperty("enabled", device->canProvision());
        rootObject->findChild<QObject*>("textMIN")->setProperty("enabled", device->canProvision());
        rootObject->findChild<QObject*>("provisionButton")->setProperty("enabled", device->canProvision());
    }

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
    device->setProvisioning(true);
    ui()->provisionProgressChanged(device, Serial::SerialProvisionStatusQueue, 0, Serial::SerialProvisionErrorNone);
    update();

    ui()->worker()->addDeviceProvision(device);

}
