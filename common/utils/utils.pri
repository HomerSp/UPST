HEADERS += \
    $$PWD/fileutils.h
SOURCES += \
    $$PWD/fileutils.cpp

win32 {
    HEADERS += \
        $$PWD/winutils.h
    SOURCES += \
        $$PWD/winutils.cpp
}
