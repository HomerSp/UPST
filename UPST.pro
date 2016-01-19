TEMPLATE = app

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

HEADERS += devicefilterevent.h \
    main.h

RESOURCES += qml.qrc \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

include(serial/serial.pri)
include(serial/qcdm/qcdm.pri)
include(serial/qcdm/commands/commands.pri)
include(serial/qcdm/commands/nv/nv.pri)

# Default rules for deployment.
include(deployment.pri)
