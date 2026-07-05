#include "iconlabelplugin.h"
#include "iconlabel.h"

IconLabelPlugin::IconLabelPlugin(QObject *parent) : QObject(parent)
{
}

void IconLabelPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core)

    if (m_initialized) {
        return;
    }

    m_initialized = true;
}

bool IconLabelPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *IconLabelPlugin::createWidget(QWidget *parent)
{
    return new IconLabel(parent);
}

QString IconLabelPlugin::includeFile() const
{
    return QStringLiteral("iconlabel.h");
}

QString IconLabelPlugin::name() const
{
    return QStringLiteral("IconLabel");
}

QString IconLabelPlugin::group() const
{
    return QStringLiteral("AstroCore widgets");
}

QIcon IconLabelPlugin::icon() const
{
    return QIcon();
}

QString IconLabelPlugin::toolTip() const
{
    return QString();
}

QString IconLabelPlugin::whatsThis() const
{
    return QString();
}

bool IconLabelPlugin::isContainer() const
{
    return false;
}

QString IconLabelPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"IconLabel\" name=\"iconLabel\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>120</width>\n"
           "    <height>24</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"text\">\n"
           "   <string>Label</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}
