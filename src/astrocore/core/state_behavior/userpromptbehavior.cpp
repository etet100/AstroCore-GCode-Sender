// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include "userpromptbehavior.h"
#include "core/communicator/communicator.h"
#include <algorithm>

UserPromptBehavior::UserPromptBehavior(PromptSpec spec, QObject *parent)
    : AbstractStateBehavior{parent}
    , m_spec(std::move(spec))
{}

QString UserPromptBehavior::description()
{
    if (!m_spec.title.isEmpty()) {
        return m_spec.title;
    }

    return QString("Prompt: %1").arg(m_spec.promptId);
}

AbstractStateBehavior::Result UserPromptBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    Q_UNUSED(communicator);
    Q_UNUSED(ctx);

    qDebug() << "[Behavior][UserPrompt] Entry — id:" << m_spec.promptId
             << "choices:" << m_spec.choices.size();
    log(QString("Awaiting user decision: %1").arg(m_spec.promptId), {"UserPrompt"});

    emit userPromptRequested(m_spec);

    return Result::Ok;
}

AbstractStateBehavior::Result UserPromptBehavior::doOnExit(AbstractStateBehavior *next)
{
    Q_UNUSED(next);
    qDebug() << "[Behavior][UserPrompt] Exit";

    return Result::Ok;
}

void UserPromptBehavior::respondToPrompt(const QString &promptId, const QString &choiceId)
{
    if (m_answered) {
        qDebug() << "[Behavior][UserPrompt] Ignoring duplicate response for" << promptId;

        return;
    }
    if (promptId != m_spec.promptId) {
        qDebug() << "[Behavior][UserPrompt] Ignoring response for unrelated prompt" << promptId
                 << "— expecting" << m_spec.promptId;

        return;
    }

    auto it = std::find_if(m_spec.choices.cbegin(), m_spec.choices.cend(),
                           [&](const PromptChoice &c) { return c.id == choiceId; });
    if (it == m_spec.choices.cend()) {
        qWarning() << "[Behavior][UserPrompt] Unknown choiceId" << choiceId
                   << "for prompt" << promptId;

        return;
    }

    m_answered = true;
    qDebug() << "[Behavior][UserPrompt] Answered:" << promptId << "->" << choiceId;
    log(QString("User chose '%1' for %2").arg(choiceId, promptId), {"UserPrompt"});

    setExitValue({
        {"promptId", promptId},
        {"choiceId", choiceId},
        {"promptContext", m_spec.context},
    });

    // Wake any coroutine that might be co_awaiting on this object too,
    // for parity with the base implementation.
    emit userPromptAnswered(promptId, choiceId);

    emit resumePrevious();
}
