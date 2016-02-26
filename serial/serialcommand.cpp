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
{

}

SerialCommand::~SerialCommand() {
    foreach(SerialCommandResult* res, mResults) {
        delete res;
    }

    mResults.clear();
}
