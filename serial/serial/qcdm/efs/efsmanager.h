#ifndef SERIAL_QCDM_EFS_EFSMANAGER_H
#define SERIAL_QCDM_EFS_EFSMANAGER_H

#include <QByteArray>

namespace Serial {
    class SerialDevice;

    namespace QCDM {
        namespace EFS {
            class EFSManager
            {
            public:
                struct FStat;

                EFSManager(SerialDevice* device);

                bool read(const QString& path, QByteArray& data);
                bool write(const QString& path, const QByteArray &data);

                bool open(const QString& path, int32_t flags, int32_t createMode, int32_t& fp);
                bool close(int32_t fp);

                bool fstat(int32_t fp, FStat& out);

                bool read(int32_t fp, uint32_t size, uint32_t offset, QByteArray& output);
                bool write(int32_t fp, uint32_t offset, const QByteArray& data);

                struct FStat {
                    int32_t mode;
                    int32_t size;
                    int32_t linkCount;
                    int32_t atime;
                    int32_t mtime;
                    int32_t ctime;
                };

            private:
                SerialDevice* mDevice;
            };
        }
    }
}

#endif // SERIAL_QCDM_EFS_EFSMANAGER_H
