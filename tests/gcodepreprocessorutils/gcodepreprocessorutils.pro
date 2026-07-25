include(../common.pri)

TARGET = tst_gcodepreprocessorutils

# The helpers themselves are static and dependency-free, but the translation
# unit also defines parseLines(), which calls GCode::operator<<, so the GCode
# data model has to be linked in. QVector3D/QMatrix4x4 need QtGui.
QT += core gui

# gcode.h must be listed so moc generates GCode's signal bodies.
HEADERS += \
    $$ASTROCORE_SRC/core/gcode/parser/gcodepreprocessorutils.h \
    $$ASTROCORE_SRC/core/gcode/gcode.h \
    $$ASTROCORE_SRC/core/gcode/gcodeprogram.h \
    $$ASTROCORE_SRC/core/gcode/gcodecursor.h \
    $$ASTROCORE_SRC/core/gcode/gcodeitem.h

SOURCES += \
    $$ASTROCORE_SRC/core/gcode/parser/gcodepreprocessorutils.cpp \
    $$ASTROCORE_SRC/core/gcode/gcode.cpp \
    $$ASTROCORE_SRC/core/gcode/gcodeprogram.cpp \
    $$ASTROCORE_SRC/core/gcode/gcodeitem.cpp \
    tst_gcodepreprocessorutils.cpp
