#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "winutils.h"

using namespace Utils;

void WinUtils::enableIntelHack() {
    DISPLAY_DEVICE device;
    device.cb = sizeof(DISPLAY_DEVICE);
    EnumDisplayDevices(NULL, 0, &device, 0);

    QString deviceName = QString::fromWCharArray(device.DeviceString);
    if(deviceName == "Intel(R) HD Graphics 3000") {
        qWarning()<<"Enabling software OpenGL due to bug in Intel driver";
        QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL, true);
    }
}
