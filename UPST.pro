TEMPLATE = subdirs
SUBDIRS = common serial program
program.depends = common serial

# Default rules for deployment.
include(deployment.pri)
