#include <QDir>
#include <QFileInfoList>

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
