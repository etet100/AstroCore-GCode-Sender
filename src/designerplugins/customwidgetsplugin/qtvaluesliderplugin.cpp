#include "qtvaluesliderplugin.h"
#include "qtvalueslider.h"

QtValueSliderPlugin::QtValueSliderPlugin(QObject *parent)
    : QObject{parent}, m_initialized(false)
{}

bool QtValueSliderPlugin::isContainer() const
{
    return false;
}

bool QtValueSliderPlugin::isInitialized() const
{
    return m_initialized;
}

QIcon QtValueSliderPlugin::icon() const
{
    return QIcon();
}

QString QtValueSliderPlugin::domXml() const
{
    return "<ui language=\"c++\"><widget class=\"QtValueSlider\" name=\"qtValueSlider\">\n"
           " <property name=\"name\">\n"
           "  <string>Value Slider</string>\n"
           " </property>\n"
           " <property name=\"class\">QtValueSlider</property>\n"
           "</widget></ui>\n";
}

QString QtValueSliderPlugin::group() const
{
    return QStringLiteral("GPilot widgets");
}

QString QtValueSliderPlugin::includeFile() const
{
    return QStringLiteral("qtvalueslider.h");
}

QString QtValueSliderPlugin::name() const
{
    return QStringLiteral("QtValueSlider");
}

QString QtValueSliderPlugin::toolTip() const
{
    return QStringLiteral("A slider widget for selecting values");
}

QString QtValueSliderPlugin::whatsThis() const
{
    return QStringLiteral("This is a custom Qt widget that allows users to select values using a slider.");
}

QWidget *QtValueSliderPlugin::createWidget(QWidget *parent)
{
    QtValueSlider *slider = new QtValueSlider(parent);
    slider->setParent(parent);

    return slider;
}

void QtValueSliderPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core)

    if (m_initialized) return;
    m_initialized = true;
}
