#include "serialcommand.h"

using namespace Serial;

SerialCommandResult::SerialCommandResult(bool success, QVariant data, uint16_t errorCode, uint8_t diagCode, bool timedOut)
    : mSuccess(success),
    mData(data),
    mErrorCode(errorCode),
    mDiagCode(diagCode),
    mTimedOut(timedOut)
{

}

SerialCommand::SerialCommand(SerialDevice* device)
    : mDevice(device),
      mTimeout(1000),
      mOffset(0),
      mName("")
{

}

SerialCommand::~SerialCommand() {
    clearResults();
}
