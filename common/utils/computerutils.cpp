#include <QObject>

#ifdef Q_OS_WIN32
#include "winutils.h"
#endif

#include "computerutils.h"

using namespace Utils;

QString ComputerUtils::sSerialNumber = "";

void ComputerUtils::init() {
#ifdef Q_OS_WIN32
    sSerialNumber = Utils::WinUtils::serialNumber();
#endif
}

void ComputerUtils::enableIntelHack() {
#ifdef Q_OS_WIN
    WinUtils::enableIntelHack();
#endif
}
