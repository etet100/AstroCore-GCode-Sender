include(../common.pri)

TARGET = tst_config

# Config modules use QColor/QVector3D (QtGui) in their (de)serialisation
# registration, and the JSON persister/provider include QtGui headers.
# No QApplication is required at runtime.
QT += core gui

HEADERS += \
    $$ASTROCORE_SRC/core/config/module/abstractconfigurationmodule.h \
    $$ASTROCORE_SRC/core/config/module/configurationjogging.h \
    $$ASTROCORE_SRC/core/config/module/configurationmachine.h \
    $$ASTROCORE_SRC/core/config/module/configurationparser.h \
    $$ASTROCORE_SRC/core/config/persistence/abstractpersister.h \
    $$ASTROCORE_SRC/core/config/persistence/abstractprovider.h \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonpersister.h \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonprovider.h

SOURCES += \
    $$ASTROCORE_SRC/core/config/module/abstractconfigurationmodule.cpp \
    $$ASTROCORE_SRC/core/config/module/configurationjogging.cpp \
    $$ASTROCORE_SRC/core/config/module/configurationmachine.cpp \
    $$ASTROCORE_SRC/core/config/module/configurationparser.cpp \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonpersister.cpp \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonprovider.cpp \
    tst_config.cpp
