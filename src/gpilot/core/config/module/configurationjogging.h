// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONJOGGING_H
#define CONFIGURATIONJOGGING_H

#include "configurationmodule.h"
#include <QObject>

class ConfigurationJogging : public ConfigurationModule
{
    friend class frmSettings;

    Q_OBJECT
    Q_PROPERTY(double step MEMBER m_step NOTIFY changed)
    Q_PROPERTY(QStringList stepChoices MEMBER m_stepChoices NOTIFY changed)
    Q_PROPERTY(int feed MEMBER m_feed NOTIFY changed)
    Q_PROPERTY(QStringList feedChoices MEMBER m_feedChoices NOTIFY changed)
    Q_PROPERTY(bool keyboardControl MEMBER m_keyboardControl NOTIFY changed)

    public:
        explicit ConfigurationJogging(QObject *parent = nullptr);
        QString getSectionName() override { return "jogging"; }

        double jogStep() const { return m_step; }
        void setJogStep(double step) { m_step = step; emit changed(); }
        const QStringList &stepChoices() const { return m_stepChoices; }
        int jogFeed() const { return m_feed; }
        void setJogFeed(int feed) { m_feed = feed; emit changed(); }
        const QStringList &feedChoices() const { return m_feedChoices; }
        bool keyboardControl() const { return m_keyboardControl; }
        void setKeyboardControl(bool keyboardControl) { m_keyboardControl = keyboardControl; emit changed(); }

    private:
        double m_step;
        QStringList m_stepChoices;
        int m_feed;
        QStringList m_feedChoices;
        bool m_keyboardControl;
};

#endif // CONFIGURATIONJOGGING_H
