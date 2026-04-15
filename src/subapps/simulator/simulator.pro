TEMPLATE = app
CONFIG -= debug_and_release
QT = core gui widgets network
TARGET = gpilot-simulator
DESTDIR = $$OUT_PWD/../../gpilot

CONN = ../../gpilot/io/connection
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
