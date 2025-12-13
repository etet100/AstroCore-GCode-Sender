TEMPLATE = subdirs

SUBDIRS = customwidgetsplugin

# Qt Creator plugins path
install_customwidgetsplugin.path = $$QTCREATOR_PLUGINS_PATH
install_customwidgetsplugin.files = customwidgetsplugin/gpilot-customwidgets.dll

win32: {
    # SUBDIRS += joystickplugin

    # install_joystickplugin.path = ../../bin/plugins/joystick/plugins
    # install_joystickplugin.files = joystickplugin/joystickplugin.dll

    INSTALLS += install_customwidgetsplugin
}

# CONFIG += c++17
# QMAKE_CXXFLAGS += -std=c++17
