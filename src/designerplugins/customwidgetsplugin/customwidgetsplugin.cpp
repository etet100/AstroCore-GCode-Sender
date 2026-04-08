#include "customwidgetsplugin.h"
#include "colorpickerplugin.h"
#include "iconlabelplugin.h"
#include "sliderplugin.h"
#include "sliderboxplugin.h"
#include "styledtoolbuttonplugin.h"
#include "qtvaluesliderplugin.h"
#include "xswitchbuttonplugin.h"
#include "qtoolbuttonwithmenuplugin.h"
#include "xswitchbuttonwithlabelplugin.h"

CustomWidgetsPlugin::CustomWidgetsPlugin(QObject *parent): QObject(parent)
{
    widgets.append(new ColorPickerPlugin(this));
    widgets.append(new IconLabelPlugin(this));
    widgets.append(new SliderPlugin(this));
    widgets.append(new SliderBoxPlugin(this));
    widgets.append(new StyledToolButtonPlugin(this));
    widgets.append(new QtValueSliderPlugin(this));
    widgets.append(new XSwitchButtonWithLabelPlugin(this));
    widgets.append(new QToolButtonWithMenuPlugin(this));
    widgets.append(new XSwitchButtonPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetsPlugin::customWidgets() const
{
    return widgets;
}
