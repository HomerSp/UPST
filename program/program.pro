TEMPLATE = app
TARGET = UPST

QT += qml quick serialport
CONFIG += c++11

RESOURCES += \
    resources.qrc \
    icons.qrc \
    qml.qrc

SOURCES += main.cpp \
    devicefilterevent.cpp

win32 {
    SOURCES += devicefilterevent_win.cpp
    HEADERS += devicefilterevent_win.h
    LIBS += -lsetupapi
} else {
    DEFINES += NATIVEFILTER_MISSING=1
}

HEADERS += devicefilterevent.h

win32: RC_FILE = res/program.rc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += ../common ../serial
LIBS += -lcommon -lserial

CONFIG( debug, debug|release ) {
    # debug
    LIBS += -L"../common/debug" -L"../serial/debug"
    PRE_TARGETDEPS += ../common/debug/libcommon.a ../serial/debug/libserial.a
} else {
    # release
    LIBS += -L"../common/release" -L"../serial/release"
    PRE_TARGETDEPS += ../common/release/libcommon.a ../serial/release/libserial.a
}

include(../defines.pri)
include(ui/ui.pri)
include(ui/guide/guide.pri)
include(ui/section/section.pri)
include(ui/worker/worker.pri)
