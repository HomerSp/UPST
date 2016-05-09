#ifndef UI_SECTION_PROVISION_H
#define UI_SECTION_PROVISION_H

#include "../ui.h"

namespace UI {
    namespace Section {
        class Provision : public UISection
        {
            Q_OBJECT
        public:
            Provision(MainUI* ui);
            ~Provision();

            virtual void beforeDeviceChanged();
            virtual void update();

        protected slots:
            void provision();
        };
    }
}

#endif // UI_SECTION_PROVISION_H
