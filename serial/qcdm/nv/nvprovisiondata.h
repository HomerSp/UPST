#ifndef NVPROVISIONDATA_H
#define NVPROVISIONDATA_H

#include "../../serialprovisiondata.h"

namespace Serial {
    namespace QCDM {
        namespace Nv {
            class NvProvisionData : public SerialProvisionData
            {
            public:
                NvProvisionData(SerialDevice* device, const QString& data);
                ~NvProvisionData();

            protected:
                virtual Serial::SerialCommand* getCommand(const QString& parent, const QString& name, const QJsonValue& jsonValue);

                virtual void update(const QString& data);
            };
        }
    }
}

#endif // NVPROVISIONDATA_H
