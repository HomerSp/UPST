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

# Remove this on production builds
DEFINES += TESTING_MODE

HEADERS += devicefilterevent.h

RESOURCES += qml.qrc \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

include(serial/serial.pri)
include(serial/qcdm/qcdm.pri)
include(serial/qcdm/commands/commands.pri)
include(serial/qcdm/commands/nvcommands/nvcommands.pri)
include(serial/qcdm/commands/nvcommands/nvprovisioncommands/nvprovisioncommands.pri)
include(serial/qcdm/nv/nv.pri)
include(ui/ui.pri)
include(ui/section/section.pri)
include(web/web.pri)

# Default rules for deployment.
include(deployment.pri)
