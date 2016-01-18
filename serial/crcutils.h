#ifndef CRCUTILS_H
#define CRCUTILS_H

#include <QList>

namespace Serial {
    class CRCUtils
    {
    public:
        CRCUtils();

        static bool addCRC(QByteArray &data);
        static int verifyCRC(const QByteArray &data);

    private:
        static bool checkByte(uint8_t &result, uint8_t chkByte);
        static void computeCRC(uint16_t &crc, uint8_t data);
    };
}

#endif // CRCUTILS_H
