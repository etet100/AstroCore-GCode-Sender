# Unit-test suites for AstroCore.
# Build all:   qmake tests/tests.pro && make
# Run all:     make check   (each suite is a Qt Test executable)
TEMPLATE = subdirs

SUBDIRS = \
    gcode \
    gcodeprogram \
    gcodecursor \
    grblparsers \
    config \
    persistence

# `make check` recurses into subdirs and runs each suite's test target.
CONFIG += ordered
