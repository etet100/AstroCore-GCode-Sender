include(../common.pri)

TARGET = tst_gcode

# GCode is a QObject facade (needs moc) over the plain GCodeProgram/GCodeCursor.
# No UI/hardware deps, so we compile its translation units straight in.
QT += core

HEADERS += \
    $$ASTROCORE_SRC/core/gcode/gcode.h \
    $$ASTROCORE_SRC/core/gcode/gcodeprogram.h \
    $$ASTROCORE_SRC/core/gcode/gcodecursor.h \
    $$ASTROCORE_SRC/core/gcode/gcodeitem.h

SOURCES += \
    $$ASTROCORE_SRC/core/gcode/gcode.cpp \
    $$ASTROCORE_SRC/core/gcode/gcodeprogram.cpp \
    $$ASTROCORE_SRC/core/gcode/gcodeitem.cpp \
    tst_gcode.cpp
