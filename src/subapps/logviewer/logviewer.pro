TEMPLATE = app
CONFIG -= debug_and_release
QT = core gui widgets network
TARGET = logviewer
DESTDIR = $$OUT_PWD/../../gpilot

include(../../vendor/vedis.pri)

SOURCES += \
    logreceiver.cpp \
    main.cpp \
    frmlog.cpp \
    cache.cpp

HEADERS += \
    frmlog.h \
    cache.h \
    logreceiver.h

FORMS += \
    frmlog.ui

