#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QSysInfo>
#include <QSettings>

#include <windows.h>
#include <shellapi.h>

#include "winutils.h"

using namespace Utils;

void WinUtils::initTaskScheduler(const QString& user) {
    QStringList params1;
    params1 << (qgetenv("WINDIR") + "\\Tasks") << "/C" << "/E" << "/P" << (user + ":F");

    QProcess cacls1Process;
    if(QSysInfo::currentCpuArchitecture() == "x86_64") {
        cacls1Process.start(qgetenv("WINDIR") + "\\sysnative\\cacls.exe", params1);
    } else {
        cacls1Process.start(qgetenv("WINDIR") + "\\system32\\cacls.exe", params1);
    }
    cacls1Process.waitForFinished();

    QStringList params2;
    params2 << (qgetenv("WINDIR") + "\\system32\\Tasks") << "/C" << "/E" << "/P" << (user + ":F");

    QProcess cacls2Process;
    if(QSysInfo::currentCpuArchitecture() == "x86_64") {
        cacls2Process.start(qgetenv("WINDIR") + "\\sysnative\\cacls.exe", params2);
    } else {
        cacls2Process.start(qgetenv("WINDIR") + "\\system32\\cacls.exe", params2);
    }
    cacls2Process.waitForFinished();
}

void WinUtils::enableIntelHack() {
    QSettings upstSettings(QCoreApplication::applicationDirPath() + "/UPST.ini", QSettings::IniFormat);
    if(upstSettings.contains("Program/SoftwareRendering")) {
        if(upstSettings.value("Program/SoftwareRendering", 0).toInt() == 1) {
            qWarning()<<"Enabling software OpenGL due to setting";
            QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL, true);
        }

        return;
    }

    DISPLAY_DEVICE device;
    device.cb = sizeof(DISPLAY_DEVICE);
    EnumDisplayDevices(NULL, 0, &device, 0);

    QString deviceName = QString::fromWCharArray(device.DeviceString);
    if(deviceName == "Intel(R) HD Graphics 3000") {
        qWarning()<<"Enabling software OpenGL due to bug in Intel driver";
        QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL, true);
    }
}

void WinUtils::createUpdaterTask() {
    QProcess schtasksProcess;
    QStringList params;
    params << "/create" << "/tn" << TASK_NAME << "/xml" << QCoreApplication::applicationDirPath() + "/Updater.xml";

    schtasksProcess.start("schtasks", params);
    schtasksProcess.waitForFinished();
}

void WinUtils::elevateUpdaterTask(const QString& user, const QString& password) {
    QProcess schtasksProcess;
    QStringList params;
    params << "/change" << "/tn" << TASK_NAME << "/rl" << "highest" << "/RU" << user << "/RP" << password << "/IT";

    schtasksProcess.start("schtasks", params);
    schtasksProcess.waitForFinished();
}

bool WinUtils::taskExists(const QString& taskName) {
    QProcess taskProcess;
    QStringList params;
    params << "/query" << "/tn" << taskName;

    taskProcess.start("schtasks", params);
    taskProcess.waitForFinished();

    QString output(taskProcess.readAllStandardError());
    return output.length() == 0 && !output.startsWith("ERROR");
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

bool Utils::WinUtils::executeElevated(const QString &path, const QStringList &argumentsList, const QString &workingDir, bool wait) {
    QString arguments = "";
    foreach(const QString& arg, argumentsList) {
        if(arguments.size() > 0) {
            arguments += " ";
        }

        arguments += "\"" + arg + "\"";
    }

    return executeElevated(path, arguments, workingDir, wait);
}

bool Utils::WinUtils::executeElevated(const QString &path, const QString &arguments, const QString &workingDir, bool wait) {
    wchar_t* wFile = new wchar_t[path.size() + 1];
    memset(wFile, 0x0, sizeof(wchar_t) * (path.size() + 1));
    path.toWCharArray(wFile);

    wchar_t* wArgs = new wchar_t[arguments.size() + 1];
    memset(wArgs, 0x0, sizeof(wchar_t) * (arguments.size() + 1));
    arguments.toWCharArray(wArgs);

    wchar_t* wDir = new wchar_t[workingDir.size() + 1];
    memset(wDir, 0x0, sizeof(wchar_t) * (workingDir.size() + 1));
    workingDir.toWCharArray(wDir);

    if(wait) {
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
    } else {
        ::ShellExecuteW(0, L"runas", wFile, wArgs, wDir, SW_SHOWNORMAL);
    }

    delete [] wFile;
    delete [] wArgs;
    delete [] wDir;

    return true;
}
