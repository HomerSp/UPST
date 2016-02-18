#include "serialcommand.h"

using namespace Serial;

SerialCommand::SerialCommand(SerialDevice* device)
    : mDevice(device),
      mTimeout(1000)
{

}

