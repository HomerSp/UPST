#ifndef UTILS_WINUTILS_H
#define UTILS_WINUTILS_H

#include <QString>

namespace Utils {
    class WinUtils
    {
    public:
        static void initTaskScheduler(const QString& user);

        // INTEL GRAPHICS BUGS !!!111one
        static void enableIntelHack();

        static void createUpdaterTask();
        static void elevateUpdaterTask(const QString& user, const QString& password);

        static bool executeElevated(const QString &path, const QStringList &argumentsList, const QString &workingDir, bool wait = true);
        static bool executeElevated(const QString &path, const QString &arguments, const QString &workingDir, bool wait = true);

        static QString serialNumber();

    private:
        WinUtils();

    };
}

#endif // UTILS_WINUTILS_H
