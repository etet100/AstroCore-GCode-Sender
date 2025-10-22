lessThan(QT_MAJOR_VERSION, 6) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 6.8 or newer")
}
equals(QT_MAJOR_VERSION, 6):lessThan(QT_MINOR_VERSION, 8) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 6.8 or newer")
}

QT = core gui opengl serialport uitools network qml
QT += multimedia multimediawidgets

VERSION=1.0.0.0

# DEFINES += DEBUG_UCNC_COMMUNICATION=1
# DEFINES += DEBUG_GRBL_COMMUNICATION=1
# DEFINES += DEBUG_RAW_TCP_COMMUNICATION=1
# QT_DEBUG_PLUGINS=1
# DEFINES += QT_DEBUG_PLUGINS=1
# DEFINES += USE_GLWINDOW

# Threat missing return warnings as errors
QMAKE_CXXFLAGS += -Werror=return-type

win32: {
    DEFINES += WINDOWS

    # QMAKE_CXXFLAGS_DEBUG += -g3 -pg
    # QMAKE_LFLAGS_DEBUG += -pg -lgmon

    # Wyłączamy inkrementację numeru buildu w CI
    !equals(DISABLE_BUILD_NUMBER_INCREMENT, "1") {
        build_nr.commands = python $$PWD/../../scripts/build_inc.py $$PWD
        build_nr.depends = FORCE

        QMAKE_EXTRA_TARGETS += build_nr
        PRE_TARGETDEPS += build_nr
    }

    HEADERS  += build.h
}

unix:!macx {
    DEFINES += UNIX LINUX #GL_GLEXT_PROTOTYPES
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/libs\'"
}

DEFINES += GLES
contains(QT_CONFIG, opengles.) {
    warning("GL ES detected. VAO will be disabled.")
    DEFINES += GLES
    INSTALLS += target
    target.path = /home/pi
}

TARGET = GPilot
TEMPLATE = app

RC_ICONS = ui/images/gpilot.ico

DEFINES += sNan=\"65536\"

TRANSLATIONS += translations/candle_en.ts translations/candle_ru.ts translations/candle_es.ts translations/candle_fr.ts translations/candle_pt.ts

# QMAKE_CXXFLAGS += /std:c++20
# QMAKE_MSC_VER = 1929
# CMAKE_CXX_STANDARD=20

win32-msvc*: {
    QMAKE_CXXFLAGS += /std:c++20
    CMAKE_CXX_STANDARD=20
}
QMAKE_MSC_VER = 1929
CONFIG += c++20
GCC_COMPILE_FLAGS = -std=c++20
win32-msvc*: {
    # QMAKE_CXXFLAGS += -std=c++20
} else {
    # CONFIG += c++17
    # QMAKE_CXXFLAGS += -std=c++17
}

# don't create both debug and release folders
CONFIG -= debug_and_release

SOURCES += main.cpp\
    core/communicator/communicator.cpp \
    core/communicator/communicator_processing_response.cpp \
    core/communicator/communicator_utils.cpp \
    core/config/configuration.cpp \
    core/config/module/configurationconnection.cpp \
    core/config/module/configurationconsole.cpp \
    core/config/module/configurationheightmap.cpp \
    core/config/module/configurationjogging.cpp \
    core/config/module/configurationmachine.cpp \
    core/config/module/configurationmodule.cpp \
    core/config/module/configurationparser.cpp \
    core/config/module/configurationsender.cpp \
    core/config/module/configurationui.cpp \
    core/config/module/configurationvisualizer.cpp \
    core/config/persistence/ini/inipersister.cpp \
    core/config/persistence/ini/iniprovider.cpp \
    core/config/persistence/json/jsonpersister.cpp \
    core/config/persistence/json/jsonprovider.cpp \
    core/config/persistence/xml/xmlpersister.cpp \
    core/config/persistence/xml/xmlprovider.cpp \
    core/gcode/converter/applyheightmap.cpp \
    core/gcode/converter/arcstolines.cpp \
    core/gcode/converter/converter.cpp \
    core/gcode/pipeline.cpp \
    core/heightmap/exporter/heightmapexporter.cpp \
    core/heightmap/loader/heightmaploader.cpp \
    core/jogger/jogger.cpp \
    core/machine/machineconfigurationparser.cpp \
    io/connection/connection.cpp \
    io/connection/connectionmanager.cpp \
    io/connection/rawtcpconnection.cpp \
    io/connection/serialconnection.cpp \
    io/connection/virtualgrblconnection.cpp \
    io/connection/virtualucncconnection.cpp \
    core/heightmap/heightmapinterpolator.cpp \
    state_behaviour/action.cpp \
    state_behaviour/connectingbehavior.cpp \
    state_behaviour/gotobehavior.cpp \
    state_behaviour/reconnectingbehavior.cpp \
    state_behaviour/resetbehavior.cpp \
    ui/drawers/cubedrawer.cpp \
    ui/drawers/cursordrawer.cpp \
    ui/drawers/tablesurfacedrawer.cpp \
    ui/drawers/vertexdataexporter.cpp \
    ui/forms/partials/main/partmainconsole.cpp \
    ui/forms/partials/main/partmaincontrol.cpp \
    ui/forms/partials/main/partmainheightmap.cpp \
    ui/forms/partials/main/partmainjog.cpp \
    ui/forms/partials/main/partmainoverride.cpp \
    ui/forms/partials/main/partmainspindle.cpp \
    ui/forms/partials/main/partmainstate.cpp \
    ui/forms/partials/main/partmainstatelcd.cpp \
    ui/forms/partials/main/partmainvirtualsettings.cpp \
    ui/forms/partials/settings/partsettingscolors.cpp \
    ui/forms/partials/settings/partsettingsconsole.cpp \
    ui/forms/partials/settings/partsettingsjogging.cpp \
    ui/forms/partials/settings/partsettingssender.cpp \
    ui/forms/partials/settings/partsettingsshortcuts.cpp \
    ui/forms/partials/settings/partsettingsvisualizer.cpp \
    ui/forms/frmgrblconfigurator.cpp \
    ui/forms/frmmain.cpp \
    ui/forms/frmsettings.cpp \
    ui/forms/frmabout.cpp \
    ui/drawers/gcodedrawer.cpp \
    ui/drawers/heightmapborderdrawer.cpp \
    ui/drawers/heightmapgriddrawer.cpp \
    ui/drawers/heightmapinterpolationdrawer.cpp \
    ui/drawers/origindrawer.cpp \
    ui/drawers/shaderdrawable.cpp \
    ui/drawers/tooldrawer.cpp \
    ui/drawers/machineboundsdrawer.cpp \
    core/gcode/gcode.cpp \
    core/gcode/exporter/gcodeexporter.cpp \
    core/gcode/loader/gcodeloader.cpp \
    core/gcode/loader/gcodethreadedloader.cpp \
    core/heightmap/heightmap.cpp \
    core/machine/machineconfiguration.cpp \
    modules/camera/camera.cpp \
    # module/camera/qvideoframeconversionhelper.cpp \
    # module/camera/videosurface.cpp \
    # module/camera/viewfinder.cpp \
    modules/camera/cameraframeprocessor.cpp \
    modules/pendant/pendant.cpp \
    core/gcode/parser/arcproperties.cpp \
    core/gcode/parser/gcodeparser.cpp \
    core/gcode/parser/gcodepreprocessorutils.cpp \
    core/gcode/parser/gcodeviewparser.cpp \
    core/gcode/parser/linesegment.cpp \
    core/gcode/parser/pointsegment.cpp \
    state_behaviour/alarmbehavior.cpp \
    # state_behaviour/checkmodebehavior.cpp \
    state_behaviour/homingbehavior.cpp \
    state_behaviour/idlebehavior.cpp \
    state_behaviour/initializationbehavior.cpp \
    state_behaviour/joggingbehavior.cpp \
    # state_behaviour/pausebehavior.cpp \
    # state_behaviour/probingbehavior.cpp \
    state_behaviour/runningbehavior.cpp \
    # state_behaviour/state.cpp \
    # state_behaviour/statealarm.cpp \
    state_behaviour/statebehavior.cpp \
    # state_behaviour/statecheckmode.cpp \
    # state_behaviour/stateconnecting.cpp \
    # state_behaviour/stateerror.cpp \
    # state_behaviour/statehoming.cpp \
    # state_behaviour/stateidle.cpp \
    # state_behaviour/stateinitialization.cpp \
    # state_behaviour/statejogging.cpp \
    # state_behaviour/statejoggingwaitingforidle.cpp \
    # state_behaviour/statepause.cpp \
    # state_behaviour/stateprobing.cpp \
    # state_behaviour/staterunning.cpp \
    # state_behaviour/statetoolchange.cpp \
    # state_behaviour/toolchangebehavior.cpp \
    ui/tables/gcodeitemdelegate.cpp \
    ui/tables/gcodetablemodel.cpp \
    ui/tables/heightmaptablemodel.cpp \
    utils/utils.cpp \
    ui/widgets/combobox.cpp \
    ui/widgets/comboboxkey.cpp \
    ui/widgets/glcontainer.cpp \
    ui/widgets/glframebuffer.cpp \
    ui/widgets/glpalette.cpp \
    ui/widgets/glwidget.cpp \
    ui/drawers/selectiondrawer.cpp \
    scripting/scriptvars.cpp \
    ui/widgets/dropwidget.cpp \
    ui/widgets/glzminmax.cpp \
    ui/widgets/qpushbuttonwithmenu.cpp

HEADERS  += ui/forms/frmmain.h \
    core/communicator/communicator.h \
    core/config/configuration.h \
    core/config/implementations.h \
    core/config/module/configurationconnection.h \
    core/config/module/configurationconsole.h \
    core/config/module/configurationheightmap.h \
    core/config/module/configurationjogging.h \
    core/config/module/configurationmachine.h \
    core/config/module/configurationmodule.h \
    core/config/module/configurationparser.h \
    core/config/module/configurationsender.h \
    core/config/module/configurationui.h \
    core/config/module/configurationvisualizer.h \
    core/config/persistence/json/jsonpersister.h \
    core/config/persistence/json/jsonprovider.h \
    core/config/persistence/persister.h \
    core/config/persistence/provider.h \
    core/config/persistence/ini/inipersister.h \
    core/config/persistence/ini/iniprovider.h \
    core/config/persistence/xml/xmlpersister.h \
    core/config/persistence/xml/xmlprovider.h \
    core/config/registry.h \
    core/gcode/converter/applyheightmap.h \
    core/gcode/converter/arcstolines.h \
    core/gcode/converter/converter.h \
    core/gcode/pipeline.h \
    core/heightmap/exporter/heightmapexporter.h \
    core/heightmap/loader/heightmaploader.h \
    core/jogger/jogger.h \
    core/machine/machineconfigurationparser.h \
    io/connection/connection.h \
    io/connection/connectionmanager.h \
    io/connection/rawtcpconnection.h \
    io/connection/serialconnection.h \
    io/connection/virtualgrblconnection.h \
    io/connection/virtualucncconnection.h \
    core/heightmap/heightmapinterpolator.h \
    state_behaviour/action.h \
    state_behaviour/connectingbehavior.h \
    state_behaviour/gotobehavior.h \
    state_behaviour/reconnectingbehavior.h \
    state_behaviour/resetbehavior.h \
    ui/drawers/cube.h \
    ui/drawers/cubedrawer.h \
    ui/drawers/cursordrawer.h \
    ui/drawers/tablesurfacedrawer.h \
    ui/drawers/vertexdataexporter.h \
    ui/forms/partials/main/partmainconsole.h \
    ui/forms/partials/main/partmaincontrol.h \
    ui/forms/partials/main/partmainheightmap.h \
    ui/forms/partials/main/partmainjog.h \
    ui/forms/partials/main/partmainoverride.h \
    ui/forms/partials/main/partmainspindle.h \
    ui/forms/partials/main/partmainstate.h \
    ui/forms/partials/main/partmainstatelcd.h \
    ui/forms/partials/main/partmainvirtualsettings.h \
    ui/forms/partials/settings/partsettingscolors.h \
    ui/forms/partials/settings/partsettingsconsole.h \
    ui/forms/partials/settings/partsettingsjogging.h \
    ui/forms/partials/settings/partsettingssender.h \
    ui/forms/partials/settings/partsettingsshortcuts.h \
    ui/forms/partials/settings/partsettingsvisualizer.h \
    ui/forms/frmgrblconfigurator.h \
    ui/forms/frmsettings.h \
    ui/forms/frmabout.h \
    ui/drawers/gcodedrawer.h \
    ui/drawers/heightmapborderdrawer.h \
    ui/drawers/heightmapgriddrawer.h \
    ui/drawers/heightmapinterpolationdrawer.h \
    ui/drawers/origindrawer.h \
    ui/drawers/shaderdrawable.h \
    ui/drawers/tooldrawer.h \
    ui/drawers/machineboundsdrawer.h \
    core/gcode/gcode.h \
    core/gcode/exporter/gcodeexporter.h \
    core/gcode/loader/gcodeloader.h \
    core/gcode/loader/gcodethreadedloader.h \
    core/globals.h \
    core/heightmap/heightmap.h \
    core/machine/machineconfiguration.h \
    modules/camera/camera.h \
    # module/camera/qvideoframeconversionhelper.h \
    # module/camera/videosurface.h \
    # module/camera/viewfinder.h \
    modules/camera/cameraframeprocessor.h \
    modules/pendant/pendant.h \
    core/gcode/parser/arcproperties.h \
    core/gcode/parser/gcodeparser.h \
    core/gcode/parser/gcodepreprocessorutils.h \
    core/gcode/parser/gcodeviewparser.h \
    core/gcode/parser/linesegment.h \
    core/gcode/parser/pointsegment.h \
    state_behaviour/alarmbehavior.h \
    state_behaviour/behaviors.h \
    # state_behaviour/checkmodebehavior.h \
    state_behaviour/homingbehavior.h \
    state_behaviour/idlebehavior.h \
    state_behaviour/initializationbehavior.h \
    state_behaviour/joggingbehavior.h \
    # state_behaviour/pausebehavior.h \
    # state_behaviour/probingbehavior.h \
    state_behaviour/runningbehavior.h \
    # state_behaviour/state.h \
    # state_behaviour/statealarm.h \
    state_behaviour/statebehavior.h \
    # state_behaviour/statecheckmode.h \
    # state_behaviour/stateconnecting.h \
    # state_behaviour/stateerror.h \
    # state_behaviour/statehoming.h \
    # state_behaviour/stateidle.h \
    # state_behaviour/stateinitialization.h \
    # state_behaviour/statejogging.h \
    # state_behaviour/statejoggingwaitingforidle.h \
    # state_behaviour/statepause.h \
    # state_behaviour/stateprobing.h \
    # state_behaviour/staterunning.h \
    # state_behaviour/states.h \
    # state_behaviour/statetoolchange.h \
    state_behaviour/toolchangebehavior.h \
    ui/tables/gcodeitemdelegate.h \
    ui/tables/gcodetablemodel.h \
    ui/tables/heightmaptablemodel.h \
    utils.h \
    utils/interpolation.h \
    utils/utils.h \
    utils/validators.h \
    ui/widgets/combobox.h \
    ui/widgets/comboboxkey.h \
    ui/widgets/glcontainer.h \
    ui/widgets/glframebuffer.h \
    ui/widgets/glpalette.h \
    ui/widgets/glwidget.h \
    ui/drawers/selectiondrawer.h \
    scripting/scriptvars.h \
    ui/widgets/dropwidget.h \
    ui/widgets/glzminmax.h \
    ui/widgets/qpushbuttonwithmenu.h

FORMS    += ui/forms/frmmain.ui \
    ui/forms/partials/main/partmainconsole.ui \
    ui/forms/partials/main/partmaincontrol.ui \
    ui/forms/partials/main/partmainheightmap.ui \
    ui/forms/partials/main/partmainjog.ui \
    ui/forms/partials/main/partmainoverride.ui \
    ui/forms/partials/main/partmainspindle.ui \
    ui/forms/partials/main/partmainstate.ui \
    ui/forms/partials/main/partmainstatelcd.ui \
    ui/forms/partials/main/partmainvirtualsettings.ui \
    ui/forms/partials/settings/partsettingscolors.ui \
    ui/forms/partials/settings/partsettingsconsole.ui \
    ui/forms/partials/settings/partsettingsjogging.ui \
    ui/forms/partials/settings/partsettingssender.ui \
    ui/forms/partials/settings/partsettingsshortcuts.ui \
    ui/forms/partials/settings/partsettingsvisualizer.ui \
    ui/forms/frmdebug.ui \
    ui/forms/frmgrblconfigurator.ui \
    ui/forms/frmsettings.ui \
    ui/forms/frmabout.ui \
    modules/camera/camera.ui

DEFINES += _USE_MATH_DEFINES

RESOURCES += \
    ui/resources/fonts.qrc \
    ui/resources/shaders.qrc \
    ui/resources/images.qrc \
    ui/resources/stylesheets.qrc

INCLUDEPATH += ../designerplugins/customwidgetsplugin
INCLUDEPATH += ../vendor/PropertyEditor

include(../vendor/CRC.pri)
include(../vendor/PropertyEditor/PropertyEditor.pri)
include(../vendor/phantomstyle/src/phantom/phantom.pri)

LIBS += -L../designerplugins/customwidgetsplugin -lcustomwidgets

# qtPrepareTool(LRELEASE, lrelease)
# for(tsfile, TRANSLATIONS) {
#     qmfile = $$tsfile
#     qmfile ~= s,.ts$,.qm,
#     qmdir = $$dirname(qmfile)
#     !exists($$qmdir) {
#         mkpath($$qmdir)|error("Aborting.")
#     }
#     command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
#     system($$command)|error("Failed to run: $$command")
# }

# INCLUDEPATH += $$PWD/../vendor/uCNC/build
# INCLUDEPATH += $$PWD/../vendor/uCNC/makefiles/virtual
# DEPENDPATH += ../vendor/uCNC

#LIBS += C:/Projekty/Qt/Candle/src/vendor/build/uCNC-Debug

LIBS += -L../vendor/uCNC -luCNC
LIBS += -L../vendor/grblHal -lgrblHal

DISTFILES += \
    shaders/2dcopy_fragment.glsl \
    shaders/2dcopy_vertex.glsl \
    shaders/base_fragment.glsl \
    shaders/base_vertex.glsl \
    shaders/cube_fragment.glsl \
    shaders/cube_vertex.glsl \
    shaders/gcode_fragment.glsl \
    shaders/gcode_vertex.glsl

