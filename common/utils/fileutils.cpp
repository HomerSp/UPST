#include <QDir>
#include <QFileInfoList>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

#include "fileutils.h"

using namespace Utils;

FileUtils::FileUtils() {

}

void Utils::FileUtils::listFiles(QList<QString> &files, const QString& source, const QString& sub) {
    QFileInfoList list = QDir(source + "/" + sub).entryInfoList();
    foreach(QFileInfo info, list) {
        QString fileName = info.fileName();
        if(fileName == ".." || fileName == ".") {
            continue;
        }

        if(info.isDir()) {
            if(sub.size() > 0) {
                listFiles(files, source, sub + "/" + fileName);
            } else {
                listFiles(files, source, fileName);
            }

            continue;
        }

        if(sub.size() > 0) {
            files.append(sub + "/" + fileName);
        } else {
            files.append(fileName);
        }
    }
}

void Utils::FileUtils::listDirs(QList<QString> &dirs, const QString& source, const QString& sub) {
    QFileInfoList list = QDir(source + "/" + sub).entryInfoList();
    foreach(QFileInfo info, list) {
        QString fileName = info.fileName();
        if(fileName == ".." || fileName == ".") {
            continue;
        }

        if(info.isDir()) {
            if(sub.size() > 0) {
                listDirs(dirs, source, sub + "/" + fileName);
                dirs.append(sub + "/" + fileName);
            } else {
                listDirs(dirs, source, fileName);
                dirs.append(fileName);
            }

            continue;
        }
    }
}

bool Utils::FileUtils::execute(const QString &path, const QStringList &argumentsList, const QString &workingDir) {
#ifdef Q_OS_WIN
    wchar_t* file = new wchar_t[path.size() + 1];
    memset(file, 0x0, sizeof(wchar_t) * (path.size() + 1));
    path.toWCharArray(file);

    QString arguments = "";
    foreach(const QString& arg, argumentsList) {
        if(arguments.size() > 0) {
            arguments += " ";
        }

        arguments += "\"" + arg + "\"";
    }

    qDebug()<<"Running program"<<path<<"with arguments"<<arguments;

    wchar_t* args = new wchar_t[arguments.size() + 1];
    memset(args, 0x0, sizeof(wchar_t) * (arguments.size() + 1));
    arguments.toWCharArray(args);

    wchar_t* dir = new wchar_t[workingDir.size() + 1];
    memset(dir, 0x0, sizeof(wchar_t) * (workingDir.size() + 1));
    workingDir.toWCharArray(dir);

    ::ShellExecuteW(0, L"runas", file, args, dir, SW_SHOWNORMAL);

    delete [] file;
    delete [] args;
    delete [] dir;

    return true;
#else
    return QProcess::startDetached(path, argumentsList, workingDir);
#endif
}
