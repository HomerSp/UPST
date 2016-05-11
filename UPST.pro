TEMPLATE = subdirs
SUBDIRS = common serial program updater
program.depends = common serial
updater.depends = common

# Default rules for deployment.
include(deployment.pri)
