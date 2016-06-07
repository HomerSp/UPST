#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>

#include <windows.h>
#include <shellapi.h>

#include "winutils.h"

using namespace Utils;

void WinUtils::initTaskScheduler(const QString& user) {
    QStringList params1;
    params1 << (qgetenv("WINDIR") + "\\Tasks") << "/C" << "/E" << "/P" << (user + ":F");

    QProcess::execute("cacls", params1);

    QStringList params2;
    params2 << (qgetenv("WINDIR") + "\\system32\\Tasks") << "/C" << "/E" << "/P" << (user + ":F");

    QProcess::execute("cacls", params2);
}

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

void WinUtils::enableUpdaterTask(const QString& user, const QString& password) {
    QProcess schtasksProcess;
    QStringList params;
    params << "/create" << "/tn" << "UPSTUpdater" << "/xml" << QString(QCoreApplication::applicationDirPath() + "/Updater.xml") << "/RU" << user << "/RP" << password;

    schtasksProcess.start("schtasks", params);
    schtasksProcess.waitForFinished();
}

void WinUtils::elevateUpdaterTask(const QString& user, const QString& password) {
    QProcess schtasksProcess;
    QStringList params;
    params << "/change" << "/tn" << "UPSTUpdater" << "/rl" << "highest" << "/RU" << user << "/RP" << password;

    schtasksProcess.start("schtasks", params);
    schtasksProcess.waitForFinished();
}

QString Utils::WinUtils::serialNumber() {
    QProcess wmicProcess;
    QStringList params;
    params << "bios" << "get" << "SerialNumber";

    wmicProcess.start("wmic", params);
    wmicProcess.waitForFinished();

    QString output(wmicProcess.readAllStandardOutput());
    if(!output.startsWith("SerialNumber")) {
        return "";
    }

    output.remove(0, 12);
    return output.trimmed();
}

bool Utils::WinUtils::executeElevated(const QString &path, const QStringList &argumentsList, const QString &workingDir) {
    QString arguments = "";
    foreach(const QString& arg, argumentsList) {
        if(arguments.size() > 0) {
            arguments += " ";
        }

        arguments += "\"" + arg + "\"";
    }

    return executeElevated(path, arguments, workingDir);
}

bool Utils::WinUtils::executeElevated(const QString &path, const QString &arguments, const QString &workingDir) {
    wchar_t* wFile = new wchar_t[path.size() + 1];
    memset(wFile, 0x0, sizeof(wchar_t) * (path.size() + 1));
    path.toWCharArray(wFile);

    wchar_t* wArgs = new wchar_t[arguments.size() + 1];
    memset(wArgs, 0x0, sizeof(wchar_t) * (arguments.size() + 1));
    arguments.toWCharArray(wArgs);

    wchar_t* wDir = new wchar_t[workingDir.size() + 1];
    memset(wDir, 0x0, sizeof(wchar_t) * (workingDir.size() + 1));
    workingDir.toWCharArray(wDir);

    SHELLEXECUTEINFOW info;
    memset(&info, 0, sizeof(SHELLEXECUTEINFOW));
    info.cbSize = sizeof(SHELLEXECUTEINFOW);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.lpVerb = L"runas";
    info.lpFile = wFile;
    info.lpParameters = wArgs;
    info.lpDirectory = wDir;
    info.nShow = SW_SHOW;
    ::ShellExecuteExW(&info);
    ::WaitForSingleObject(info.hProcess, INFINITE);
    ::CloseHandle(info.hProcess);

    delete [] wFile;
    delete [] wArgs;
    delete [] wDir;

    return true;
}
