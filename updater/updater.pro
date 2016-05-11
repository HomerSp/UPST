TEMPLATE = app
TARGET = Updater

QT += qml quick
CONFIG += c++11

RESOURCES += \
    resources.qrc \
    qml.qrc

SOURCES += main.cpp
HEADERS +=

win32: RC_FILE = res/program.rc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

INCLUDEPATH += ../common
LIBS += -lcommon

include(../defines.pri)
include(ui/ui.pri)
