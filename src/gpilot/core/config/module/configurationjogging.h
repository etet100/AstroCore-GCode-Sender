// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONJOGGING_H
#define CONFIGURATIONJOGGING_H

#include "configurationmodule.h"
#include <QObject>

class ConfigurationJogging : public ConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(double step MEMBER m_step NOTIFY changed)
    Q_PROPERTY(QStringList stepChoices MEMBER m_stepChoices NOTIFY changed)
    Q_PROPERTY(int feed MEMBER m_feed NOTIFY changed)
    Q_PROPERTY(int feedz MEMBER m_feedz NOTIFY changed)
    Q_PROPERTY(QStringList feedChoices MEMBER m_feedChoices NOTIFY changed)
    Q_PROPERTY(bool keyboardControl MEMBER m_keyboardControl NOTIFY changed)
    Q_PROPERTY(bool sepFeedZ MEMBER m_sepFeedZ NOTIFY changed)

    public:
        explicit ConfigurationJogging(QObject *parent = nullptr);
        QString getSectionName() override { return "jogging"; }

        double step() const { return m_step; }
        void setStep(double step) { m_step = step; emit changed(); }
        const QStringList &stepChoices() const { return m_stepChoices; }
        int feed() const { return m_feed; }
        int feedZ() const { return m_feedz; }
        int finalFeedZ() const { return m_sepFeedZ ? m_feedz : m_feed; }
        void setFeed(int feed) { m_feed = feed; emit changed(); }
        void setFeedZ(int feedz) { m_feedz = feedz; emit changed(); }
        const QStringList &feedChoices() const { return m_feedChoices; }
        bool keyboardControl() const { return m_keyboardControl; }
        void setKeyboardControl(bool keyboardControl) { m_keyboardControl = keyboardControl; emit changed(); }
        bool separateFeedZ() const { return m_sepFeedZ; }
        void setSeparateFeedZ(bool sepFeedZ) { m_sepFeedZ = sepFeedZ; emit changed(); }

    private:
        double m_step;
        bool m_sepFeedZ;
        QStringList m_stepChoices;
        int m_feed;
        int m_feedz;
        QStringList m_feedChoices;
        bool m_keyboardControl;
};

#endif // CONFIGURATIONJOGGING_H
