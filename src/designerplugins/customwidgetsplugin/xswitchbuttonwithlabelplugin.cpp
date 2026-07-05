#include "xswitchbuttonwithlabelplugin.h"
#include "xswitchbuttonwithlabel.h"

XSwitchButtonWithLabelPlugin::XSwitchButtonWithLabelPlugin(QObject *parent) : QObject(parent)
{
}

bool XSwitchButtonWithLabelPlugin::isContainer() const
{
    return false;
}

bool XSwitchButtonWithLabelPlugin::isInitialized() const
{
    return m_initialized;
}

QIcon XSwitchButtonWithLabelPlugin::icon() const
{
    return QIcon();
}

QString XSwitchButtonWithLabelPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"XSwitchButtonWithLabel\" name=\"xSwitchButtonWithLabel\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>150</width>\n"
           "    <height>30</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"text\">\n"
           "   <string>Label</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString XSwitchButtonWithLabelPlugin::group() const
{
    return QStringLiteral("AstroCore widgets");
}

QString XSwitchButtonWithLabelPlugin::includeFile() const
{
    return QStringLiteral("xswitchbuttonwithlabel.h");
}

QString XSwitchButtonWithLabelPlugin::name() const
{
    return QStringLiteral("XSwitchButtonWithLabel");
}

QString XSwitchButtonWithLabelPlugin::toolTip() const
{
    return QStringLiteral("Switch button with a text label");
}

QString XSwitchButtonWithLabelPlugin::whatsThis() const
{
    return QStringLiteral("A mobile-style switch button with a label on the right side.");
}

QWidget *XSwitchButtonWithLabelPlugin::createWidget(QWidget *parent)
{
    return new XSwitchButtonWithLabel(parent);
}

void XSwitchButtonWithLabelPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core)

    m_initialized = true;
}
