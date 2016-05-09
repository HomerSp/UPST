PROG_VERSION=0.9.0
DEFINES += PROG_VERSION=\\\"$$PROG_VERSION\\\"
VERSION=$$PROG_VERSION

DEFINES += LOG_LIMIT=10000

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
