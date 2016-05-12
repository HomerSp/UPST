#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <QEventLoop>
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>

namespace Utils {
    class FileUtils
    {
    public:
        static void listFiles(QList<QString> &files, const QString& source, const QString& sub = QString());
        static void listDirs(QList<QString> &dirs, const QString& source, const QString& sub = QString());

        static bool execute(const QString& path, const QStringList& argumentsList, const QString& workingDir);

    private:
        FileUtils();
    };
}

#endif // COMMON_UTILS_H
