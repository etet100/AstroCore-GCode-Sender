#include "xswitchbuttonplugin.h"
#include "xswitchbutton.h"

XSwitchButtonPlugin::XSwitchButtonPlugin(QObject *parent) : QObject(parent), m_initialized(false)
{
}

bool XSwitchButtonPlugin::isContainer() const
{
    return false;
}

bool XSwitchButtonPlugin::isInitialized() const
{
    return m_initialized;
}

QIcon XSwitchButtonPlugin::icon() const
{
    return QIcon();
}

QString XSwitchButtonPlugin::domXml() const
{
    return QStringLiteral(
        "<ui language=\"c++\"><widget class=\"XSwitchButton\" name=\"xSwitchButton\">\n"
        " <property name=\"geometry\">\n"
        "  <rect>\n"
        "   <x>0</x>\n"
        "   <y>0</y>\n"
        "   <width>60</width>\n"
        "   <height>30</height>\n"
        "  </rect>\n"
        " </property>\n"
        " <property name=\"minimumSize\">\n"
        "   <size>\n"
        "    <width>50</width>\n"
        "    <height>30</height>\n"
        "   </size>\n"
        " </property>\n"
        " <property name=\"class\">XSwitchButton</property>\n"
        "</widget></ui>\n"
    );
}

QString XSwitchButtonPlugin::group() const
{
    return QStringLiteral("GPilot widgets");
}

QString XSwitchButtonPlugin::includeFile() const
{
    return QStringLiteral("xswitchbutton.h");
}

QString XSwitchButtonPlugin::name() const
{
    return QStringLiteral("XSwitchButton");
}

QString XSwitchButtonPlugin::toolTip() const
{
    return QStringLiteral("Mobile style switch button");
}

QString XSwitchButtonPlugin::whatsThis() const
{
    return QStringLiteral("A switch button styled like those found in mobile operating systems.");
}

QWidget *XSwitchButtonPlugin::createWidget(QWidget *parent)
{
    return new XSwitchButton(parent);
}

void XSwitchButtonPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core)

    m_initialized = true;
}
