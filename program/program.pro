TEMPLATE = app
TARGET = UPST

QT += qml quick serialport
CONFIG += c++11

SOURCES += main.cpp \
    devicefilterevent.cpp

win32 {
    SOURCES += devicefilterevent_win.cpp
    LIBS += -lsetupapi
} else {
    DEFINES += NATIVEFILTER_MISSING=1
}

HEADERS += devicefilterevent.h

RESOURCES += \
    resources.qrc \
    icons.qrc \
    qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += ../common ../serial
LIBS += -lcommon -lserial

include(../defines.pri)
include(ui/ui.pri)
include(ui/section/section.pri)
