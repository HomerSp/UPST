#ifndef SECCODECOMMAND_H
#define SECCODECOMMAND_H

#include "../nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                namespace Provision {
                    class SecCodeCommand : public NvCommand
                    {
                    public:
                        SecCodeCommand(SerialDevice* device, bool read = true, const QJsonValue* jsonValue = nullptr)
                            : NvCommand(device, read, QCDM::NV_SEC_CODE_I, jsonValue->toString().toLatin1())
                        {
                            if(!read) {
                                mDevice = device;
                                mNewSPC = jsonValue->toString();
                            } else {
                                mDevice = nullptr;
                            }
                        }

                        virtual void execute(SerialDevice* device, bool obeyOffset = true) {
                            QcdmCommand::execute(device, obeyOffset);

                            if(mDevice != nullptr) {
                                mDevice->updateSPC(mNewSPC);
                            }
                        }

                    private:
                        SerialDevice* mDevice;
                        QString mNewSPC;
                    };
                }
            }
        }
    }
}

#endif // SECCODECOMMAND_H
