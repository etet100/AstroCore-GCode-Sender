CONFIG += plugin shared
CONFIG -= debug_and_release
TEMPLATE = lib
QT += widgets uiplugin multimedia multimediawidgets

TARGET = gpilot-customwidgets
QMAKE_FLAGS += -Wl,-soname,libgpilot-customwidgets.so
DEFINES += CUSTOMWIDGETS_EXPORT

HEADERS +=  colorpicker.h \
            customwidgetsshared.h \
            qtvalueslider.h \
            qtvaluesliderplugin.h \
            slider.h \
            colorpickerplugin.h \
            sliderplugin.h \
            sliderbox.h \
            sliderboxplugin.h \
            styledtoolbutton.h \
            styledtoolbuttonplugin.h \
            customwidgetsplugin.h \
            xswitchbutton.h \
            xswitchbuttonplugin.h

SOURCES +=  colorpicker.cpp \
            qtvalueslider.cpp \
            qtvaluesliderplugin.cpp \
            slider.cpp \
            colorpickerplugin.cpp \
            sliderplugin.cpp \
            sliderbox.cpp \
            sliderboxplugin.cpp \
            styledtoolbutton.cpp \
            styledtoolbuttonplugin.cpp \
            customwidgetsplugin.cpp \
            xswitchbutton.cpp \
            xswitchbuttonplugin.cpp

FORMS +=    sliderbox.ui

include(../../vendor/QtValueSlider.pri)

# Install plugin to Qt Creator plugins directory
# Use environment variable or qmake argument to override, e.g.:
# qmake QTCREATOR_PLUGINS_PATH="C:/custom/path"
isEmpty(QTCREATOR_PLUGINS_PATH) {
    QTCREATOR_PLUGINS_PATH = C:/Programy/Qt/Tools/QtCreator/bin/plugins/designer
}
target.path = $$QTCREATOR_PLUGINS_PATH
INSTALLS += target
