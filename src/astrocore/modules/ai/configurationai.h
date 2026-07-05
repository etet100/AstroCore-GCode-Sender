#ifndef CONFIGURATIONAI_H
#define CONFIGURATIONAI_H

#include <QObject>
#include "core/config/module/abstractconfigurationmodule.h"

class Configuration;

class ConfigurationAI : public AbstractConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(QString openAIKey MEMBER m_openAIKey NOTIFY changed)

    public:
        explicit ConfigurationAI(QObject *parent = nullptr);
        ConfigurationAI& operator=(const ConfigurationAI&) { return *this; }
        QString getSectionName() override { return "ai"; }

        QString openAIKey() const { return m_openAIKey; }
        void setOpenAIKey(const QString& key) { m_openAIKey = key; }

        // Global accessor for the single AI config instance. Owned by the
        // singleton itself; registers with Configuration via registerWith().
        static ConfigurationAI& instance();
        static void registerWith(Configuration& cfg);

    private:
        QString m_openAIKey;
};

#endif // CONFIGURATIONAI_H
