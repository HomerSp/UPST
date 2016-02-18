#ifndef SERIALCOMMAND_H
#define SERIALCOMMAND_H

#include <QByteArray>

#include "serialdevice.h"

namespace Serial {
    class SerialCommunicator;
    class SerialDevice;

    class SerialCommand
    {
    public:
        virtual bool execute(QList<QByteArray>& result, uint16_t* errorCode = nullptr) = 0;
        virtual bool execute(QByteArray& result, uint16_t* errorCode = nullptr) = 0;

        int timeout() const {
            return mTimeout;
        }

        void setTimeout(int timeout) {
            mTimeout = timeout;
        }

    protected:
        SerialCommand(SerialDevice *device);

        SerialCommunicator* communicator() {
            return mDevice->communicator();
        }

    private:
        SerialDevice* mDevice;

        int mTimeout;
    };
}
#endif // SERIALCOMMAND_H
