include(../common.pri)

TARGET = tst_grblparsers

# These parsers use QVector3D (QtGui) but need no QApplication.
QT += core gui

SOURCES += \
    $$ASTROCORE_SRC/core/communicator/statusreportprocessor.cpp \
    $$ASTROCORE_SRC/core/communicator/proberesponseparser.cpp \
    $$ASTROCORE_SRC/core/machine/modalstateparser.cpp \
    tst_grblparsers.cpp
