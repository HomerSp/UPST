#ifndef SERIALCOMMAND_H
#define SERIALCOMMAND_H

#include <QByteArray>
#include <QVariant>
#include <QList>

#include <cstdint>

#include "serialdevice.h"

namespace Serial {
    class SerialCommand;
    class SerialCommunicator;

    class SerialCommandResult {
    public:
        SerialCommandResult(bool success, QVariant data, uint16_t errorCode);

        bool success() const {
            return mSuccess;
        }

        const QVariant &data() const {
            return mData;
        }

        uint16_t errorCode() const {
            return mErrorCode;
        }

        void setSuccess(bool success) {
            mSuccess = success;
        }

        void setData(QVariant data) {
            mData = data;
        }

        void setErrorCode(uint16_t err) {
            mErrorCode = err;
        }

    private:
        bool mSuccess;
        QVariant mData;
        uint16_t mErrorCode;
    };

    class SerialCommand
    {
    public:
        virtual ~SerialCommand();

        virtual void execute() = 0;

        int timeout() const {
            return mTimeout;
        }

        SerialCommandResult* result(int i = 0) {
            return mResults.at(i);
        }

        const QList<SerialCommandResult*> &results() {
            return mResults;
        }

        bool resultSuccess(int i = 0) {
            if(i < 0 || i >= mResults.size()) {
                return false;
            }

            return mResults.at(i)->success();
        }

        void setTimeout(int timeout) {
            mTimeout = timeout;
        }

    protected:
        SerialCommand(SerialDevice *device);

        SerialCommunicator* communicator() {
            return mDevice->communicator();
        }

        void addResult(bool success, QVariant data = 0, uint16_t errorCode = 0) {
            mResults.append(new SerialCommandResult(success, data, errorCode));
        }

    private:
        SerialDevice* mDevice;

        int mTimeout;

        QList<SerialCommandResult*> mResults;
    };
}
#endif // SERIALCOMMAND_H
