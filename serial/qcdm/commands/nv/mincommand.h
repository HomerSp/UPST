#ifndef MINCOMMAND_H
#define MINCOMMAND_H

#include "nvcommand.h"

namespace Serial {
    namespace QCDM {
        namespace Commands {
            namespace Nv {
                class MINCommand : public NvCommand
                {
                public:
                    MINCommand(SerialDevice* device, bool read = true, uint64_t data = 0);

                    virtual bool execute(uint64_t &result, uint16_t* errorCode = nullptr);

                private:
                    bool fromNv(const QList<QByteArray>& data, uint64_t& result);
                };
            }
        }
    }
}

#endif // MINCOMMAND_H
