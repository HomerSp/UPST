#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QThread>

#include <windows.h>
#include <dbt.h>
#include <devguid.h> // for GUID_DEVCLASS_PORTS and GUID_DEVCLASS_MODEM
#include <winioctl.h> // for GUID_DEVINTERFACE_COMPORT
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <initguid.h>

#include "devicefilterevent.h"
#include "devicefilterevent_win.h"

DEFINE_GUID(GUID_DEVINTERFACE_COMPORT,0x86e0d1e0,0x8089,0x11d0,0x9c,0xe4,0x08,0x00,0x3e,0x30,0x1f,0x73);

DeviceFilterEventWin::DeviceFilterEventWin()
    : DeviceFilterEvent()
{

}

bool DeviceFilterEventWin::nativeEventFilter(const QByteArray &eventType, void* message, long*) {
    if(eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG *msg = (MSG*)message;
        if(msg->message == WM_DEVICECHANGE) {
            if(msg->wParam == DBT_DEVICEARRIVAL || msg->wParam == DBT_DEVICEREMOVECOMPLETE) {
                process();
            }
        }
    }

    return false;
}

QSet<QString> DeviceFilterEventWin::getDevices() {
    QSet<QString> ret;

    HDEVINFO deviceInfoSet = ::SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(deviceInfoSet == INVALID_HANDLE_VALUE) {
        return ret;
    }

    SP_DEVINFO_DATA deviceInfoData;
    ::memset(&deviceInfoData, 0, sizeof(SP_DEVINFO_DATA));
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    DWORD index = 0;
    while(::SetupDiEnumDeviceInfo(deviceInfoSet, index++, &deviceInfoData)) {
        const HKEY key = ::SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if(key == INVALID_HANDLE_VALUE) {
            continue;
        }

        wchar_t data[256];
        memset(data, 0, 256);

        DWORD size = 255;
        if(::RegQueryValueEx(key, L"PortName", nullptr, nullptr, reinterpret_cast<PBYTE>(data), &size) == ERROR_SUCCESS) {
            QString portName = QString::fromWCharArray(data);
            if(!portName.isEmpty()) {
                ret.insert(portName);
            }
        }

        ::RegCloseKey(key);
    }

    return ret;
}
