// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef USERPROMPTBEHAVIOR_H
#define USERPROMPTBEHAVIOR_H

#include "abstractstatebehavior.h"

// Generic "stop and ask the user" state. The parent suspends itself
// (TransitionKind::Suspend) and pushes a UserPromptBehavior carrying the
// PromptSpec; the user's choice arrives back through EntryContext::data:
//   - "promptId":      QString — same as spec.promptId
//   - "choiceId":      QString — id of the picked PromptChoice
//   - "promptContext": QVariantMap — verbatim copy of spec.context
//
// Disconnect / Reset remain available so the user can always escape.
class UserPromptBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        explicit UserPromptBehavior(PromptSpec spec, QObject *parent = nullptr);

        QString description() override;
        Type type() const override { return Type::UserPrompt; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Reset, Action::Disconnect };
        }

        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(AbstractStateBehavior *next) override;

        void respondToPrompt(const QString &promptId, const QString &choiceId) override;

        const PromptSpec& spec() const { return m_spec; }

    protected:
        QString name() const override { return "UserPrompt"; }

    private:
        PromptSpec m_spec;
        bool m_answered = false;
};

#endif // USERPROMPTBEHAVIOR_H
