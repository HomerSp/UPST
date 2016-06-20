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
    QString spc = rootObject->findChild<QObject*>("textSPC")->property("text").toString();

    currentDevice()->setProvisionData(mdn, min, spc);
}

void UI::Section::Provision::update() {
    UISection::startUpdate();

    Serial::SerialDevice* device = currentDevice();
    if(device != nullptr) {
        QObject* rootObject = UISection::rootObject();
        rootObject->findChild<QObject*>("textMDN")->setProperty("text", device->newMdnStr());
        rootObject->findChild<QObject*>("textMIN")->setProperty("text", device->newMinStr());
        rootObject->findChild<QObject*>("textSPC")->setProperty("text", device->spc());

        rootObject->findChild<QObject*>("textMDN")->setProperty("enabled", device->canProvision());
        rootObject->findChild<QObject*>("textMIN")->setProperty("enabled", device->canProvision());
        rootObject->findChild<QObject*>("textSPC")->setProperty("enabled", device->canProvision());
        rootObject->findChild<QObject*>("spcLayout")->setProperty("visible", device->wrongSPC());
        rootObject->findChild<QObject*>("provisionButton")->setProperty("enabled", device->canProvision());
    }

    UISection::endUpdate();
}

void UI::Section::Provision::saveChanges() {
    beforeDeviceChanged();
}

void UI::Section::Provision::provision() {
    Serial::SerialDevice* device = currentDevice();
    if(device == nullptr || !device->canProvision()) {
        return;
    }

    beforeDeviceChanged();

    qDebug()<<"Provision"<<device->name();

    device->setProvisioning(true);
    ui()->provisionProgressChanged(device, Serial::SerialProvisionStatusQueue, 0, Serial::SerialProvisionErrorNone);
    update();

    ui()->deviceWorker()->addDeviceProvision(device);

}
