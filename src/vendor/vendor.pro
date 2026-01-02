TEMPLATE = subdirs

SUBDIRS = PropertyEditor

win32 {
        SUBDIRS += grblHal \
        uCNC
}

HEADERS += Arduino.h

DISTFILES += CRC.pri QtValueSlider.pri

