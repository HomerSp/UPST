#ifndef UI_SECTION_MANUAL_H
#define UI_SECTION_MANUAL_H

#include "../ui.h"

namespace UI {
    namespace Section {
        class Manual : public UISection
        {
            Q_OBJECT
        public:
            Manual(MainUI* ui);
            ~Manual();

            virtual void update();

        protected slots:
            void rawSend();
            void rawCommandFinished();
        };
    }
}

#endif // UI_SECTION_MANUAL_H
