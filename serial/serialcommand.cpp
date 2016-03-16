#include "serialcommand.h"

using namespace Serial;

SerialCommandResult::SerialCommandResult(bool success, QVariant data, uint16_t errorCode)
    : mSuccess(success),
    mData(data),
    mErrorCode(errorCode)
{

}

SerialCommand::SerialCommand(SerialDevice* device)
    : mDevice(device),
      mTimeout(1000)
#ifdef TESTING_MODE
    ,mDebuggingName("")
#endif
{

}

SerialCommand::~SerialCommand() {
    clearResults();
}
