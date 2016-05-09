TEMPLATE = lib
TARGET = serial
DESTDIR = serial
CONFIG += staticlib

QT += qml quick serialport
CONFIG += c++11

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += ../common
LIBS += -lcommon

include(../defines.pri)
include(serial/serial.pri)
include(serial/qcdm/qcdm.pri)
include(serial/qcdm/commands/commands.pri)
include(serial/qcdm/commands/nvcommands/nvcommands.pri)
include(serial/qcdm/commands/nvcommands/nvprovisioncommands/nvprovisioncommands.pri)
include(serial/qcdm/nv/nv.pri)