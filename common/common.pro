TEMPLATE = lib
TARGET = common
CONFIG += staticlib

QT += qml quick serialport
CONFIG += c++11

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

RESOURCES += \
    common.qrc

include(../defines.pri)
include(web/web.pri)
