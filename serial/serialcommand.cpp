#include "serialcommand.h"

using namespace Serial;

SerialCommand::SerialCommand(SerialCommunicator* communicator)
    : mCommunicator(communicator),
      mTimeout(1000)
{

}

