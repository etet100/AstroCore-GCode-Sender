# Shared settings for every unit-test suite.
CONFIG += c++20 console testcase
CONFIG -= app_bundle
QT += testlib

# Root of the application sources, so test files can include headers as
# "core/gcode/gcode.h" exactly like the app does.
ASTROCORE_SRC = $$PWD/../src/astrocore
INCLUDEPATH += $$ASTROCORE_SRC

# Match the app: treat missing returns as errors.
QMAKE_CXXFLAGS += -Werror=return-type
