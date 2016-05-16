#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <windows.h>
#include <shellapi.h>

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

bool Utils::WinUtils::execute(const QString &path, const QStringList &argumentsList, const QString &workingDir) {
    QString operation = "runas";
    QString tmpPath = path;

    QString arguments = "";
    foreach(const QString& arg, argumentsList) {
        if(arguments.size() > 0) {
            arguments += " ";
        }

        arguments += "\\\"" + arg + "\\\"";
    }

    QString adminUser = getAdminUser();
    if(adminUser.length() > 0) {
        operation = "";
        tmpPath = "runas";
        arguments = "/user:" + adminUser + " /savecred \"\\\"" + path + "\\\" " + arguments + "\"";
    }

    qDebug()<<"Running program"<<path<<"with arguments"<<arguments;

    wchar_t* wOperation = new wchar_t[operation.size() + 1];
    memset(wOperation, 0x0, sizeof(wchar_t) * (operation.size() + 1));
    operation.toWCharArray(wOperation);

    wchar_t* wFile = new wchar_t[tmpPath.size() + 1];
    memset(wFile, 0x0, sizeof(wchar_t) * (tmpPath.size() + 1));
    tmpPath.toWCharArray(wFile);

    wchar_t* wArgs = new wchar_t[arguments.size() + 1];
    memset(wArgs, 0x0, sizeof(wchar_t) * (arguments.size() + 1));
    arguments.toWCharArray(wArgs);

    wchar_t* wDir = new wchar_t[workingDir.size() + 1];
    memset(wDir, 0x0, sizeof(wchar_t) * (workingDir.size() + 1));
    workingDir.toWCharArray(wDir);

    ::ShellExecuteW(0, wOperation, wFile, wArgs, wDir, SW_NORMAL);

    delete [] wOperation;
    delete [] wFile;
    delete [] wArgs;
    delete [] wDir;

    return true;
}

QString Utils::WinUtils::getAdminUser() {
    QString cfgPath = QCoreApplication::applicationFilePath();
    if(!cfgPath.endsWith("exe")) {
        return "";
    }

    cfgPath.replace(cfgPath.length() - 3, 3, "cfg");

    QFile cfgFile(cfgPath);
    if(!cfgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }

    QTextStream stream(&cfgFile);
    while(!stream.atEnd()) {
        QString line = stream.readLine();
        if(line.startsWith("AdminUser")) {
            QStringList list = line.split('=');
            if(list.length() == 2) {
                return list.at(1);
            }
        }
    }

    return "";
}
