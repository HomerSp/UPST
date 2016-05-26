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
        SerialCommandResult(bool success, QVariant data, uint16_t errorCode, uint8_t diagCode);

        bool success() const {
            return mSuccess;
        }

        const QVariant &data() const {
            return mData;
        }

        uint16_t errorCode() const {
            return mErrorCode;
        }

        uint8_t diagCode() const {
            return mDiagCode;
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

        void setDiagCode(uint8_t diag) {
            mDiagCode = diag;
        }

    private:
        bool mSuccess;
        QVariant mData;
        uint16_t mErrorCode;
        uint8_t mDiagCode;
    };

    class SerialCommand
    {
    public:
        virtual ~SerialCommand();

        virtual void execute() = 0;
        virtual void execute(SerialDevice* device, bool obeyOffset = true) = 0;

        int timeout() const {
            return mTimeout;
        }

        void clearResults() {
            foreach(SerialCommandResult* res, mResults) {
                delete res;
            }

            mResults.clear();
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

        void setOffset(int offset) {
            mOffset = offset;
        }

        const QString& debuggingName() {
            return mDebuggingName;
        }

        void setDebuggingName(const QString& name) {
            mDebuggingName = name;
        }

    protected:
        SerialCommand(SerialDevice *device);

        SerialDevice* device() {
            return mDevice;
        }

        void addResult(bool success, QVariant data = 0, uint16_t errorCode = 0, uint8_t diagCode = 0) {
            mResults.append(new SerialCommandResult(success, data, errorCode, diagCode));
        }

        int offset() const {
            return mOffset;
        }

    private:
        SerialDevice* mDevice;

        int mTimeout;

        int mOffset;

        QList<SerialCommandResult*> mResults;

        QString mDebuggingName;
    };
}
#endif // SERIALCOMMAND_H
