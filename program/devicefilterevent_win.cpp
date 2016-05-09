#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <windows.h>
#include <dbt.h>
#include <setupapi.h>
#include <devguid.h>

#include "devicefilterevent.h"

bool DeviceFilterEvent::nativeEventFilter(const QByteArray &eventType, void* message, long*) {
    if(eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG *msg = (MSG*)message;
        if(msg->message == WM_DEVICECHANGE) {
            if(msg->wParam == DBT_DEVICEARRIVAL || msg->wParam == DBT_DEVICEREMOVECOMPLETE) {
                DEV_BROADCAST_HDR *pHdr = reinterpret_cast<DEV_BROADCAST_HDR*>(msg->lParam);
                if(pHdr != nullptr) {
                    qDebug() << "nativeEventFilter" << "WM_DEVICECHANGE" << msg->wParam << pHdr->dbch_devicetype;

                    if(msg->wParam == DBT_DEVICEARRIVAL) {
                        handleDeviceAdded();
                    } else {
                        if(pHdr->dbch_devicetype == DBT_DEVTYP_PORT) {
                            DEV_BROADCAST_PORT *pPortHdr = reinterpret_cast<DEV_BROADCAST_PORT*>(pHdr);
                            handleDeviceRemoved(QString::fromWCharArray(pPortHdr->dbcp_name));
                        }
                    }

                    /*if(pHdr->dbch_devicetype == DBT_DEVTYP_PORT) {
                        DEV_BROADCAST_PORT *pPortHdr = reinterpret_cast<DEV_BROADCAST_PORT*>(pHdr);
                        handleDeviceChanged(QString::fromWCharArray(pPortHdr->dbcp_name), (msg->wParam == DBT_DEVICEARRIVAL));
                    }*/
                }
            }
        }
    }

    return false;
}
