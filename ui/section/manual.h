#ifndef UI_SECTION_MANUAL_H
#define UI_SECTION_MANUAL_H

#include "../ui.h"

namespace UI {
    namespace Section {
        class Manual : public UISection
        {
        public:
            Manual(MainUI* ui);
            ~Manual();

            virtual void update();
        };
    }
}

#endif // UI_SECTION_MANUAL_H
