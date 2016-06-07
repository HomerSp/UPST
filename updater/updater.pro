TEMPLATE = app
TARGET = Updater

QT += qml quick
CONFIG += c++11

RESOURCES += \
    resources.qrc \
    qml.qrc

SOURCES += main.cpp \
    $$PWD/updateworker.cpp
HEADERS += \
    $$PWD/updateworker.h

win32: RC_FILE = res/program.rc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += ../common
LIBS += -lcommon

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
