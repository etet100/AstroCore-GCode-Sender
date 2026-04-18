lessThan(QT_MAJOR_VERSION, 6) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 6.8 or newer")
}
equals(QT_MAJOR_VERSION, 6):lessThan(QT_MINOR_VERSION, 8) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 6.8 or newer")
}

# QMAKEFEATURES += $$PWD/../../src/vendor/qcoro/install/mkspecs/modules
include($$PWD/../../src/vendor/qcoro/install/mkspecs/modules/qt_QCoroCore.pri)
LIBS += -L$$PWD/../../src/vendor/qcoro/build/qcoro/core/ -lQCoro6Core
INCLUDEPATH += $$PWD/../../src/vendor/qcoro/install/include/qcoro6/qcoro/

QT = core gui opengl serialport uitools network qml xml svg
QT += multimedia multimediawidgets
# QT += QCoro6Core

VERSION=1.0.0.0

# DEFINES += DEBUG_UCNC_COMMUNICATION=1
# DEFINES += DEBUG_GRBL_COMMUNICATION=1
# DEFINES += DEBUG_RAW_TCP_COMMUNICATION=1
# QT_DEBUG_PLUGINS=1
# DEFINES += QT_DEBUG_PLUGINS=1
# DEFINES += USE_GLWINDOW
DEFINES += VIRTUAL_SIMULATOR_PROCESS

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

CONFIG -= qml_debug

SOURCES += main.cpp\
    core/communicator/communicator.cpp \
    core/communicator/communicator_processing_response.cpp \
    core/communicator/commandbuffer.cpp \
    core/communicator/commandscanner.cpp \
    core/communicator/positiontracker.cpp \
    core/communicator/overrides.cpp \
    core/communicator/communicator_utils.cpp \
    core/communicator/statebehaviorgarbagecollector.cpp \
    core/communicator/statebehaviormanager.cpp \
    core/communicator/statusreportprocessor.cpp \
    core/communicator/proberesponseparser.cpp \
    core/config/configuration.cpp \
    core/config/module/configurationai.cpp \
    core/config/module/configurationconnection.cpp \
    core/config/module/configurationconsole.cpp \
    core/config/module/configurationheightmap.cpp \
    core/config/module/configurationjogging.cpp \
    core/config/module/configurationmachine.cpp \
    core/config/module/configurationmacros.cpp \
    core/config/module/abstractconfigurationmodule.cpp \
    core/config/module/configurationparser.cpp \
    core/config/module/configurationpendant.cpp \
    core/config/module/configurationsender.cpp \
    core/config/module/configurationui.cpp \
    core/config/module/configurationvisualizer.cpp \
    core/config/persistence/ini/inipersister.cpp \
    core/config/persistence/ini/iniprovider.cpp \
    core/config/persistence/json/jsonpersister.cpp \
    core/config/persistence/json/jsonprovider.cpp \
    core/config/persistence/xml/xmlpersister.cpp \
    core/config/persistence/xml/xmlprovider.cpp \
    core/core.cpp \
    core/gcode/converter/applyheightmap.cpp \
    core/gcode/converter/arcstolines.cpp \
    core/gcode/converter/abstractconverter.cpp \
    core/gcode/converter/exampleconverter.cpp \
    core/gcode/converter/fusionrestorerapidmovements.cpp \
    core/gcode/converter/pipeline.cpp \
    core/gcode/converter/shakinggcode.cpp \
    core/gcode/converter/singleconverter.cpp \
    core/heightmap/exporter/heightmapexporter.cpp \
    core/heightmap/interpolator/heightmapbicubicinterpolator.cpp \
    core/heightmap/interpolator/heightmapbilinearinterpolator.cpp \
    core/heightmap/interpolator/abstractheightmapinterpolator.cpp \
    core/heightmap/interpolator/heightmaplinearinterpolator.cpp \
    core/heightmap/interpolator/heightmapnearestneighbourinterpolator.cpp \
    core/heightmap/loader/heightmaploader.cpp \
    core/jogger/jogger.cpp \
    core/machine/physicalmachineconfiguration.cpp \
    core/machine/physicalmachineconfigurationparser.cpp \
    core/machine/modalstateparser.cpp \
    core/utils/filesmanager.cpp \
    core/utils/programtimeestimator.cpp \
    core/utils/timer.cpp \
    io/connection/abstractconnection.cpp \
    io/connection/connectionmanager.cpp \
    io/connection/rawtcpconnection.cpp \
    io/connection/serialconnection.cpp \
    io/connection/virtualconnection.cpp \
    io/connection/virtualfluidncconnection.cpp \
    io/connection/virtualgrblconnection.cpp \
    io/connection/virtualucncconnection.cpp \
    io/connection/virtualgrblworkerthread.cpp \
    io/connection/virtualfluidncworkerthread.cpp \
    io/connection/virtualucncworkerthread.cpp \
    core/state_behavior/action.cpp \
    core/state_behavior/checkmodebehavior.cpp \
    core/state_behavior/connectingbehavior.cpp \
    core/state_behavior/errorbehavior.cpp \
    core/state_behavior/gotobehavior.cpp \
    core/state_behavior/holdbehavior.cpp \
    core/state_behavior/probingbehavior.cpp \
    core/state_behavior/reconnectingbehavior.cpp \
    core/state_behavior/resetbehavior.cpp \
    core/state_behavior/scantablebehavior.cpp \
    core/state_behavior/scantableerrorbehavior.cpp \
    core/state_behavior/handshakebehavior.cpp \
    core/state_behavior/externalprocessbehavior.cpp \
    core/state_behavior/disconnectionbehavior.cpp \
    core/state_behavior/toolchangebehavior.cpp \
    ui/drawers/billboarddrawable.cpp \
    ui/drawers/boundingboxdrawer.cpp \
    ui/drawers/compositedrawable.cpp \
    ui/drawers/cubedrawer.cpp \
    ui/drawers/cursordrawer.cpp \
    ui/drawers/heightmapareadrawer.cpp \
    ui/drawers/originbillboarddrawer.cpp \
    ui/drawers/nogcodedefaultdrawer.cpp \
    ui/drawers/tablesurfacedrawer.cpp \
    ui/drawers/vertexdataexporter.cpp \
    ui/forms/frmclosingapp.cpp \
    ui/forms/frmlog.cpp \
    ui/forms/modals/dlgeditheightmappoint.cpp \
    ui/forms/modals/dlgeditprogram.cpp \
    ui/forms/partials/main/partmainconsole.cpp \
    ui/forms/partials/main/partmaincontrol.cpp \
    ui/forms/partials/main/partmainheightmap.cpp \
    ui/forms/partials/main/partmainjog.cpp \
    ui/forms/partials/main/partmainjogparameters.cpp \
    ui/forms/partials/main/partmainjogparameters2.cpp \
    ui/forms/partials/main/partmainjogparameters3.cpp \
    ui/forms/partials/main/partmainmacros.cpp \
    ui/forms/partials/main/partmainoverride.cpp \
    ui/forms/partials/main/partmainprogram.cpp \
    ui/forms/partials/main/partmainspindle.cpp \
    ui/forms/partials/main/partmainstate.cpp \
    ui/forms/partials/main/abstractpartmainstate.cpp \
    ui/forms/partials/main/partmainstatelcd.cpp \
    ui/forms/partials/main/partmainvirtualsettings.cpp \
    ui/forms/partials/main/partmainvisualizer.cpp \
    ui/forms/partials/settings/partsettingsai.cpp \
    ui/forms/partials/settings/partsettingscolors.cpp \
    ui/forms/partials/settings/partsettingsconsole.cpp \
    ui/forms/partials/settings/partsettingsjogging.cpp \
    ui/forms/partials/settings/partsettingspendant.cpp \
    ui/forms/partials/settings/partsettingssender.cpp \
    ui/forms/partials/settings/partsettingsshortcuts.cpp \
    ui/forms/partials/settings/partsettingsvisualizer.cpp \
    ui/forms/frmgrblconfigurator.cpp \
    ui/forms/frmmain.cpp \
    ui/forms/frmmain_uiscale.cpp \
    ui/forms/frmsettings.cpp \
    ui/forms/frmabout.cpp \
    ui/drawers/gcodedrawer.cpp \
    ui/drawers/heightmapgriddrawer.cpp \
    ui/drawers/heightmapinterpolationdrawer.cpp \
    ui/drawers/origindrawer.cpp \
    ui/drawers/shaderdrawable.cpp \
    ui/drawers/tooldrawer.cpp \
    ui/drawers/lightsourcedrawer.cpp \
    ui/drawers/machineboundsdrawer.cpp \
    core/gcode/gcode.cpp \
    core/gcode/gcodefilterview.cpp \
    core/gcode/exporter/gcodeexporter.cpp \
    core/gcode/loader/abstractgcodeloader.cpp \
    core/gcode/loader/gcodethreadedloader.cpp \
    core/heightmap/heightmap.cpp \
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
    core/gcode/parser/viewtransform/simplifyviewtransform.cpp \
    core/gcode/parser/viewtransform/heightmapviewtransform.cpp \
    core/state_behavior/alarmbehavior.cpp \
    # core/state_behavior/checkmodebehavior.cpp \
    core/state_behavior/homingbehavior.cpp \
    core/state_behavior/idlebehavior.cpp \
    core/state_behavior/initializationbehavior.cpp \
    core/state_behavior/joggingbehavior.cpp \
    core/state_behavior/pausebehavior.cpp \
    # core/state_behavior/probingbehavior.cpp \
    core/state_behavior/runningbehavior.cpp \
    core/state_behavior/abstractstatebehavior.cpp \
    # core/state_behavior/toolchangebehavior.cpp \
    ui/tables/gcodeitemdelegate.cpp \
    ui/tables/gcodetablemodel.cpp \
    ui/tables/heightmaptablemodel.cpp \
    ui/utils/flowlayout.cpp \
    ui/utils/shortcutsmanager.cpp \
    ui/utils/statecolors.cpp \
    ui/utils/syntaxhighlighter.cpp \
    ui/utils/thememanager.cpp \
    ui/utils/windowstaskbar.cpp \
    ui/widgets/dockabletitle.cpp \
    ui/widgets/filedropoverlay.cpp \
    modules/ai/openaimanager.cpp \
    utils/cache.cpp \
    utils/utils.cpp \
    ui/widgets/combobox.cpp \
    ui/widgets/comboboxkey.cpp \
    ui/widgets/glcontainer.cpp \
    ui/widgets/glframebuffer.cpp \
    ui/widgets/glpalette.cpp \
    ui/widgets/glwidget.cpp \
    ui/drawers/selectiondrawer.cpp \
    core/communicator/machinecoordinatecache.cpp \
    ui/widgets/dropwidget.cpp \
    ui/widgets/glzminmax.cpp \
    ui/widgets/macrowidget.cpp \
    ui/widgets/qpushbuttonwithmenu.cpp

HEADERS  += ui/forms/frmmain.h \
    core/communicator/communicator.h \
    core/communicator/commandbuffer.h \
    core/communicator/commandscanner.h \
    core/communicator/positiontracker.h \
    core/communicator/overrides.h \
    core/communicator/machinestatus.h \
    core/communicator/statebehaviorgarbagecollector.h \
    core/communicator/statebehaviormanager.h \
    core/communicator/statusreportprocessor.h \
    core/communicator/proberesponseparser.h \
    core/config/configuration.h \
    core/config/implementations.h \
    core/config/module/configurationai.h \
    core/config/module/configurationconnection.h \
    core/config/module/configurationconsole.h \
    core/config/module/configurationheightmap.h \
    core/config/module/configurationjogging.h \
    core/config/module/configurationmachine.h \
    core/config/module/configurationmacros.h \
    core/config/module/abstractconfigurationmodule.h \
    core/config/module/configurationparser.h \
    core/config/module/configurationpendant.h \
    core/config/module/configurationsender.h \
    core/config/module/configurationui.h \
    core/config/module/configurationvisualizer.h \
    core/config/persistence/json/jsonpersister.h \
    core/config/persistence/json/jsonprovider.h \
    core/config/persistence/abstractpersister.h \
    core/config/persistence/abstractprovider.h \
    core/config/persistence/ini/inipersister.h \
    core/config/persistence/ini/iniprovider.h \
    core/config/persistence/xml/xmlpersister.h \
    core/config/persistence/xml/xmlprovider.h \
    core/config/registry.h \
    core/core.h \
    core/gcode/converter/applyheightmap.h \
    core/gcode/converter/arcstolines.h \
    core/gcode/converter/abstractconverter.h \
    core/gcode/converter/abstractbatchconverter.h \
    core/gcode/converter/exampleconverter.h \
    core/gcode/converter/fusionrestorerapidmovements.h \
    core/gcode/converter/pipeline.h \
    core/gcode/converter/shakinggcode.h \
    core/gcode/converter/singleconverter.h \
    core/heightmap/exporter/heightmapexporter.h \
    core/heightmap/interpolator/heightmapbicubicinterpolator.h \
    core/heightmap/interpolator/heightmapbilinearinterpolator.h \
    core/heightmap/interpolator/abstractheightmapinterpolator.h \
    core/heightmap/interpolator/heightmaplinearinterpolator.h \
    core/heightmap/interpolator/heightmapnearestneighbourinterpolator.h \
    core/heightmap/loader/heightmaploader.h \
    core/jogger/jogger.h \
    core/machine/physicalmachineconfiguration.h \
    core/machine/physicalmachineconfigurationparser.h \
    core/machine/modalstateparser.h \
    core/utils/filesmanager.h \
    core/utils/programtimeestimator.h \
    core/utils/timer.h \
    io/connection/abstractconnection.h \
    io/connection/connectionmanager.h \
    io/connection/rawtcpconnection.h \
    io/connection/serialconnection.h \
    io/connection/virtualconnection.h \
    io/connection/virtualfluidncconnection.h \
    io/connection/virtualgrblconnection.h \
    io/connection/virtualucncconnection.h \
    io/connection/simulatordefs.h \
    io/connection/virtualgrblworkerthread.h \
    io/connection/virtualfluidncworkerthread.h \
    io/connection/virtualucncworkerthread.h \
    modules/pendant/circularbuffer.h \
    core/state_behavior/action.h \
    core/state_behavior/checkmodebehavior.h \
    core/state_behavior/connectingbehavior.h \
    core/state_behavior/errorbehavior.h \
    core/state_behavior/gotobehavior.h \
    core/state_behavior/holdbehavior.h \
    core/state_behavior/probingbehavior.h \
    core/state_behavior/reconnectingbehavior.h \
    core/state_behavior/resetbehavior.h \
    core/state_behavior/scantablebehavior.h \
    core/state_behavior/scantableerrorbehavior.h \
    core/state_behavior/handshakebehavior.h \
    core/state_behavior/externalprocessbehavior.h \
    core/state_behavior/disconnectionbehavior.h \
    core/state_behavior/toolchangebehavior.h \
    ui/drawers/billboarddrawable.h \
    ui/drawers/boundingboxdrawer.h \
    ui/drawers/compositedrawable.h \
    ui/drawers/cube.h \
    ui/drawers/cubedrawer.h \
    ui/drawers/cursordrawer.h \
    ui/drawers/abstractdrawable.h \
    ui/drawers/heightmapareadrawer.h \
    ui/drawers/originbillboarddrawer.h \
    ui/drawers/nogcodedefaultdrawer.h \
    ui/drawers/tablesurfacedrawer.h \
    ui/drawers/vertexdataexporter.h \
    ui/forms/frmclosingapp.h \
    ui/forms/frmlog.h \
    ui/forms/modals/dlgeditheightmappoint.h \
    ui/forms/modals/dlgeditprogram.h \
    ui/forms/partials/main/partmainconsole.h \
    ui/forms/partials/main/partmaincontrol.h \
    ui/forms/partials/main/partmainheightmap.h \
    ui/forms/partials/main/partmainjog.h \
    ui/forms/partials/main/partmainjogparameters.h \
    ui/forms/partials/main/partmainjogparameters2.h \
    ui/forms/partials/main/partmainjogparameters3.h \
    ui/forms/partials/main/abstractpartmainjogparameters.h \
    ui/forms/partials/main/partmainmacros.h \
    ui/forms/partials/main/partmainoverride.h \
    ui/forms/partials/main/partmainprogram.h \
    ui/forms/partials/main/partmainspindle.h \
    ui/forms/partials/main/partmainstate.h \
    ui/forms/partials/main/abstractpartmainstate.h \
    ui/forms/partials/main/partmainstatelcd.h \
    ui/forms/partials/main/partmainvirtualsettings.h \
    ui/forms/partials/main/partmainvisualizer.h \
    ui/forms/partials/settings/partsettingsai.h \
    ui/forms/partials/settings/partsettingscolors.h \
    ui/forms/partials/settings/partsettingsconsole.h \
    ui/forms/partials/settings/partsettingsjogging.h \
    ui/forms/partials/settings/partsettingspendant.h \
    ui/forms/partials/settings/partsettingssender.h \
    ui/forms/partials/settings/partsettingsshortcuts.h \
    ui/forms/partials/settings/partsettingsvisualizer.h \
    ui/forms/frmgrblconfigurator.h \
    ui/forms/frmsettings.h \
    ui/forms/frmabout.h \
    ui/drawers/gcodedrawer.h \
    ui/drawers/heightmapgriddrawer.h \
    ui/drawers/heightmapinterpolationdrawer.h \
    ui/drawers/origindrawer.h \
    ui/drawers/shaderdrawable.h \
    ui/drawers/tooldrawer.h \
    ui/drawers/lightsourcedrawer.h \
    ui/drawers/machineboundsdrawer.h \
    core/gcode/gcode.h \
    core/gcode/gcodefilterview.h \
    core/gcode/exporter/gcodeexporter.h \
    core/gcode/loader/abstractgcodeloader.h \
    core/gcode/loader/gcodethreadedloader.h \
    core/globals.h \
    core/heightmap/heightmap.h \
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
    core/gcode/parser/viewtransform/abstractviewtransform.h \
    core/gcode/parser/viewtransform/simplifyviewtransform.h \
    core/gcode/parser/viewtransform/heightmapviewtransform.h \
    core/state_behavior/alarmbehavior.h \
    core/state_behavior/behaviors.h \
    # core/state_behavior/checkmodebehavior.h \
    core/state_behavior/homingbehavior.h \
    core/state_behavior/idlebehavior.h \
    core/state_behavior/initializationbehavior.h \
    core/state_behavior/joggingbehavior.h \
    core/state_behavior/pausebehavior.h \
    # core/state_behavior/probingbehavior.h \
    core/state_behavior/runningbehavior.h \
    core/state_behavior/abstractstatebehavior.h \
    ui/tables/gcodeitemdelegate.h \
    ui/tables/gcodetablemodel.h \
    ui/tables/heightmaptablemodel.h \
    ui/utils/flowlayout.h \
    ui/utils/shortcutsmanager.h \
    ui/utils/statecolors.h \
    ui/utils/uipermissions.h \
    ui/utils/syntaxhighlighter.h \
    ui/utils/thememanager.h \
    ui/utils/windowstaskbar.h \
    ui/widgets/dockabletitle.h \
    ui/widgets/filedropoverlay.h \
    utils/cache.h \
    utils/interpolation.h \
    modules/ai/openaimanager.h \
    utils/utils.h \
    utils/validators.h \
    ui/widgets/combobox.h \
    ui/widgets/comboboxkey.h \
    ui/widgets/glcontainer.h \
    ui/widgets/glframebuffer.h \
    ui/widgets/glpalette.h \
    ui/widgets/glwidget.h \
    ui/drawers/selectiondrawer.h \
    core/communicator/machinecoordinatecache.h \
    ui/widgets/dropwidget.h \
    ui/widgets/glzminmax.h \
    ui/widgets/macrowidget.h \
    ui/widgets/qpushbuttonwithmenu.h

FORMS    += ui/forms/frmmain.ui \
    ui/forms/frmclosingapp.ui \
    ui/forms/frmlog.ui \
    ui/forms/modals/dlgeditheightmappoint.ui \
    ui/forms/modals/dlgeditprogram.ui \
    ui/forms/partials/main/partmainconsole.ui \
    ui/forms/partials/main/partmaincontrol.ui \
    ui/forms/partials/main/partmainheightmap.ui \
    ui/forms/partials/main/partmainjog.ui \
    ui/forms/partials/main/partmainjogparameters.ui \
    ui/forms/partials/main/partmainjogparameters2.ui \
    ui/forms/partials/main/partmainjogparameters3.ui \
    ui/forms/partials/main/partmainmacros.ui \
    ui/forms/partials/main/partmainoverride.ui \
    ui/forms/partials/main/partmainprogram.ui \
    ui/forms/partials/main/partmainspindle.ui \
    ui/forms/partials/main/partmainstate.ui \
    ui/forms/partials/main/partmainstatelcd.ui \
    ui/forms/partials/main/partmainvirtualsettings.ui \
    ui/forms/partials/main/partmainvisualizer.ui \
    ui/forms/partials/settings/partsettingsai.ui \
    ui/forms/partials/settings/partsettingscolors.ui \
    ui/forms/partials/settings/partsettingsconsole.ui \
    ui/forms/partials/settings/partsettingsjogging.ui \
    ui/forms/partials/settings/partsettingspendant.ui \
    ui/forms/partials/settings/partsettingssender.ui \
    ui/forms/partials/settings/partsettingsshortcuts.ui \
    ui/forms/partials/settings/partsettingsvisualizer.ui \
    ui/forms/frmgrblconfigurator.ui \
    ui/forms/frmsettings.ui \
    ui/forms/frmabout.ui \
    modules/camera/camera.ui \
    ui/widgets/dockabletitle.ui \
    ui/widgets/filedropoverlay.ui

DEFINES += _USE_MATH_DEFINES

RESOURCES += \
    ui/resources/fonts.qrc \
    ui/resources/shaders.qrc \
    ui/resources/images.qrc \
    ui/resources/stylesheets.qrc

INCLUDEPATH += ../designerplugins/customwidgetsplugin
INCLUDEPATH += ../vendor/PropertyEditor

include(../vendor/CRC.pri)
include(../vendor/QtValueSlider.pri)
include(../vendor/PropertyEditor/PropertyEditor.pri)
include(../vendor/phantomstyle/src/phantom/phantom.pri)
include(../vendor/vedis.pri)
include(../vendor/qcoro.pri)

LIBS += -L. -lgpilot-customwidgets

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

LIBS += -luuid
win32 {
    LIBS += -lole32 -loleaut32  -luser32 -lshell32
    LIBS += -L../vendor/uCNC -luCNC
    LIBS += -L../vendor/grblHal -lgrblHal
}

DISTFILES += \
    shaders/2dcopy_fragment.glsl \
    shaders/2dcopy_vertex.glsl \
    shaders/base_fragment.glsl \
    shaders/base_vertex.glsl \
    shaders/cube_fragment.glsl \
    shaders/cube_vertex.glsl \
    shaders/gcode_fragment.glsl \
    shaders/gcode_vertex.glsl
