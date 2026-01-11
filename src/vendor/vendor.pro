TEMPLATE = subdirs

SUBDIRS = PropertyEditor

win32 {
        SUBDIRS += grblHal \
        uCNC \
        FluidNC
}

HEADERS += Arduino.h

DISTFILES += CRC.pri QtValueSlider.pri

