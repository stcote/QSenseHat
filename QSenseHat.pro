#-------------------------------------------------
#
# Project created by QtCreator 2016-01-20T01:00:13
#
#-------------------------------------------------

TARGET = QSenseHat

TEMPLATE = lib

DEFINES += QSENSEHAT_LIBRARY

SOURCES += QSenseHat.cpp \
           SHJoystick.cpp \
           SHLedMatrix.cpp \
           SHSensors.cpp

HEADERS += QSenseHat.h\
           qsensehat_global.h \
           SHJoystick.h \
           SHLedMatrix.h \
           SHSensors.h

unix {
    target.path = /usr/lib
    INSTALLS += target

    iTarget.path = /usr/include
    iTarget.files = *.h
    INSTALLS += iTarget
}
