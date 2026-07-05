#ifndef CONFIGURATION_MACROS_H
#define CONFIGURATION_MACROS_H

#include <QObject>
#include "abstractconfigurationmodule.h"

struct MacroItem {
    QString name;
    int type = 0;
    bool enabled = true;
    QString content;

    bool operator==(const MacroItem& other) const {
        return name == other.name && type == other.type
            && enabled == other.enabled && content == other.content;
    }
};

Q_DECLARE_METATYPE(MacroItem)
Q_DECLARE_METATYPE(QList<MacroItem>)

class ConfigurationMacros : public AbstractConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(QList<MacroItem> macros MEMBER m_macros NOTIFY changed)

    public:
        explicit ConfigurationMacros(QObject *parent = nullptr);
        ConfigurationMacros& operator=(const ConfigurationMacros&) { return *this; }
        QString getSectionName() override { return "macros"; }

        QList<MacroItem> macros() const { return m_macros; }
        void setMacros(const QList<MacroItem>& macros) { m_macros = macros; emit changed(); }

    private:
        QList<MacroItem> m_macros;
};

#endif // CONFIGURATION_MACROS_H
