include(../common.pri)

TARGET = tst_persistence

# JSON/INI providers need QtCore (QJson, QSettings); XML needs QtXml (QDom).
# The sources also include QtGui headers (unused at runtime).
QT += core gui xml

HEADERS += \
    $$ASTROCORE_SRC/core/config/persistence/abstractprovider.h \
    $$ASTROCORE_SRC/core/config/persistence/abstractpersister.h \
    $$ASTROCORE_SRC/core/config/persistence/ini/iniprovider.h \
    $$ASTROCORE_SRC/core/config/persistence/ini/inipersister.h \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonprovider.h \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonpersister.h \
    $$ASTROCORE_SRC/core/config/persistence/xml/xmlprovider.h \
    $$ASTROCORE_SRC/core/config/persistence/xml/xmlpersister.h

SOURCES += \
    $$ASTROCORE_SRC/core/config/persistence/ini/iniprovider.cpp \
    $$ASTROCORE_SRC/core/config/persistence/ini/inipersister.cpp \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonprovider.cpp \
    $$ASTROCORE_SRC/core/config/persistence/json/jsonpersister.cpp \
    $$ASTROCORE_SRC/core/config/persistence/xml/xmlprovider.cpp \
    $$ASTROCORE_SRC/core/config/persistence/xml/xmlpersister.cpp \
    tst_persistence.cpp
