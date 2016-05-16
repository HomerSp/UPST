#include <QDir>
#include <QFileInfoList>
#include <QHostInfo>

#ifdef Q_OS_WIN
#include "winutils.h"
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
    return Utils::WinUtils::execute(path, argumentsList, workingDir);
#else
    return QProcess::startDetached(path, argumentsList, workingDir);
#endif
}
