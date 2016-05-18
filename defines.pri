PROG_VERSION=0.9.3
PROG_BUILDTIME=1463563619

DEFINES += PROG_VERSION=\\\"$$PROG_VERSION\\\" PROG_BUILDTIME=\\\"$$PROG_BUILDTIME\\\"
VERSION=$$PROG_VERSION

DEFINES += LOG_LIMIT=1000000

# Remove this on production builds
DEFINES += TESTING_MODE

CONFIG( debug, debug|release ) {
    # debug
    LIBS += -L"../common/debug" -L"../serial/debug"
    PRE_TARGETDEPS += ../common/debug/libcommon.a ../serial/debug/libserial.a
} else {
    # release
    LIBS += -L"../common/release" -L"../serial/release"
    PRE_TARGETDEPS += ../common/release/libcommon.a ../serial/release/libserial.a
}
