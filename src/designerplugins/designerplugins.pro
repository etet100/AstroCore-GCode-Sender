TEMPLATE = subdirs

SUBDIRS = customwidgetsplugin

# Qt Creator plugins path
customwidgetsplugin.path = $$QTCREATOR_PLUGINS_PATH
customwidgetsplugin.files = customwidgetsplugin/gpilot-customwidgets.dll

win32: {
    # SUBDIRS += joystickplugin

    # install_joystickplugin.path = ../../bin/plugins/joystick/plugins
    # install_joystickplugin.files = joystickplugin/joystickplugin.dll

    INSTALLS += customwidgetsplugin
}

# CONFIG += c++17
# QMAKE_CXXFLAGS += -std=c++17
