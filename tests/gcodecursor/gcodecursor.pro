include(../common.pri)

TARGET = tst_gcodecursor

# GCodeCursor is header-only over GCodeProgram; no QObject, no moc.
QT += core

SOURCES += \
    $$ASTROCORE_SRC/core/gcode/gcodeprogram.cpp \
    $$ASTROCORE_SRC/core/gcode/gcodeitem.cpp \
    tst_gcodecursor.cpp
