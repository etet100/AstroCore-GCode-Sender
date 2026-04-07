#ifndef CONFIGURATIONAI_H
#define CONFIGURATIONAI_H

#include <QObject>
#include "configurationmodule.h"

class ConfigurationAI : public ConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(QString openAIKey MEMBER m_openAIKey NOTIFY changed)

    public:
        explicit  ConfigurationAI(QObject *parent = nullptr);
        ConfigurationAI& operator=(const ConfigurationAI&) { return *this; }
        QString getSectionName() override { return "ai"; }

        QString openAIKey() const { return m_openAIKey; }
        void setOpenAIKey(const QString& key) { m_openAIKey = key; }

    private:
        QString m_openAIKey;
};

#endif // CONFIGURATIONAI_H
