include(../common.pri)

TARGET = tst_gcodeprogram

# GCodeProgram is a plain class (no QObject), so no moc and no signal layer.
QT += core

SOURCES += \
    $$ASTROCORE_SRC/core/gcode/gcodeprogram.cpp \
    $$ASTROCORE_SRC/core/gcode/gcodeitem.cpp \
    tst_gcodeprogram.cpp
