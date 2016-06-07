#ifndef COMPUTERUTILS_H
#define COMPUTERUTILS_H

#include <QString>

namespace Utils {
    class ComputerUtils
    {
    public:
        static void init();

        static void enableIntelHack();

        static const QString &serialNumber() {
            return sSerialNumber;
        }

    private:
        ComputerUtils();

        static QString sSerialNumber;
    };
}

#endif // COMPUTERUTILS_H
