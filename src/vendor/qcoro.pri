# QCoro - Coroutines for Qt (https://qcoro.dev)
# Header-only: only Task and Signal modules are used.

QCORO_DIR = $$PWD/qcoro/qcoro

INCLUDEPATH += $$QCORO_DIR $$QCORO_DIR/core

# Suppress deprecation warning for QCORO_STD macro
DEFINES += QCORO_NO_DEPRECATED_QCOROSTD
