#ifndef SERIALCOMMAND_H
#define SERIALCOMMAND_H

#include <QByteArray>

namespace Serial {
    class SerialCommunicator;

    class SerialCommand
    {
    public:
        virtual bool execute(QByteArray& result, uint16_t* errorCode = nullptr) = 0;

        int timeout() const {
            return mTimeout;
        }

        void setTimeout(int timeout) {
            mTimeout = timeout;
        }

    protected:
        SerialCommand(SerialCommunicator *communicator);

        SerialCommunicator* communicator() {
            return mCommunicator;
        }

    private:
        SerialCommunicator* mCommunicator;

        int mTimeout;
    };
}
#endif // SERIALCOMMAND_H
