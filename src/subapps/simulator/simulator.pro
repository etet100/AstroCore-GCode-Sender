TEMPLATE = app
CONFIG -= debug_and_release
QT = core gui widgets network
TARGET = astrocore-simulator
DESTDIR = $$OUT_PWD/../../astrocore

CONN = ../../astrocore/io/connection
INCLUDEPATH += $$CONN

SOURCES += \
    main.cpp \
    frmsimulator.cpp \
    $$CONN/virtualgrblworkerthread.cpp \
    $$CONN/virtualfluidncworkerthread.cpp \
    $$CONN/virtualucncworkerthread.cpp

HEADERS += \
    frmsimulator.h \
    $$CONN/simulatordefs.h \
    $$CONN/virtualgrblworkerthread.h \
    $$CONN/virtualfluidncworkerthread.h \
    $$CONN/virtualucncworkerthread.h
