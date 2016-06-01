#ifndef UTILS_WINUTILS_H
#define UTILS_WINUTILS_H

#include <QString>

namespace Utils {
    class WinUtils
    {
    public:
        // INTEL GRAPHICS BUGS !!!111one
        static void enableIntelHack();

        static bool execute(const QString& path, const QStringList& argumentsList, const QString& workingDir);

        static QString serialNumber();

    private:
        WinUtils();

        static QString getAdminUser();
    };
}

#endif // UTILS_WINUTILS_H
