#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "../ui.h"

namespace UI {
    namespace Section {
        class DeviceInfo : public UISection
        {
        public:
            DeviceInfo(MainUI* ui);
            ~DeviceInfo();

            virtual void update();

        private slots:
            void infoCommandFinished();
        };
    }
}

#endif // DEVICEINFO_H
