HEADERS += \
    $$PWD/fileutils.h \
    $$PWD/computerutils.h
SOURCES += \
    $$PWD/fileutils.cpp \
    $$PWD/computerutils.cpp

win32 {
    HEADERS += \
        $$PWD/winutils.h
    SOURCES += \
        $$PWD/winutils.cpp
}
