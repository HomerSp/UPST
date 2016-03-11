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

                    virtual void execute();

                    bool toNv(uint64_t data, QList<QByteArray>& result);

                private:
                    bool fromNv(const QList<QByteArray>& data, uint64_t& result);
                };
            }
        }
    }
}

#endif // MINCOMMAND_H
