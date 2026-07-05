#include "qtoolbuttonwithmenuplugin.h"
#include "qtoolbuttonwithmenu.h"

QToolButtonWithMenuPlugin::QToolButtonWithMenuPlugin(QObject *parent) : QObject(parent)
{
}

void QToolButtonWithMenuPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core)

    if (m_initialized) {
        return;
    }

    m_initialized = true;
}

bool QToolButtonWithMenuPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *QToolButtonWithMenuPlugin::createWidget(QWidget *parent)
{
    return new QToolButtonWithMenu(parent);
}

QString QToolButtonWithMenuPlugin::includeFile() const
{
    return QStringLiteral("qtoolbuttonwithmenu.h");
}

QString QToolButtonWithMenuPlugin::name() const
{
    return QStringLiteral("QToolButtonWithMenu");
}

QString QToolButtonWithMenuPlugin::group() const
{
    return QStringLiteral("AstroCore widgets");
}

QIcon QToolButtonWithMenuPlugin::icon() const
{
    return QIcon();
}

QString QToolButtonWithMenuPlugin::toolTip() const
{
    return QStringLiteral("A tool button with a separate menu indicator in the corner");
}

QString QToolButtonWithMenuPlugin::whatsThis() const
{
    return QString();
}

bool QToolButtonWithMenuPlugin::isContainer() const
{
    return false;
}

QString QToolButtonWithMenuPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"QToolButtonWithMenu\" name=\"toolButtonWithMenu\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>80</width>\n"
           "    <height>40</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"text\">\n"
           "   <string>Button</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}
