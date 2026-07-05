// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef USERPROMPTSPEC_H
#define USERPROMPTSPEC_H

#include <QMetaType>
#include <QString>
#include <QList>
#include <QVariantMap>

// Declarative description of a "stop and ask the user" prompt.
//
// Behaviors raise prompts in two ways:
//   1. transition(new UserPromptBehavior(spec), TransitionKind::Suspend)
//      — the parent behavior is suspended; the user's choice arrives via
//      EntryContext::data on resume (choiceId, promptId, context).
//   2. co_await askUser(spec) inside a QCoro task — the current behavior
//      stays active and the choiceId is returned as the coroutine result.
//
// Both paths emit the same userPromptRequested signal up to the UI and
// expect Communicator::respondToPrompt(promptId, choiceId) to come back.

struct PromptChoice {
    // Stable identifier returned to the parent behavior. Use lowercase
    // single words: "resume", "abort", "skip", "retry".
    QString id;

    // User-facing label.
    QString label;

    // True for choices that may discard work (Abort, Skip line). UI uses
    // this to style the button differently.
    bool destructive = false;

    // True for the choice that should be triggered by Enter / default
    // activation. At most one choice should set this.
    bool isDefault = false;
};

struct PromptSpec {
    // Stable identifier of *what* is being asked. Use dotted scope.id
    // form: "scan.move-failed", "running.command-error",
    // "toolchange.confirm". Used both for routing answers back and for
    // future telemetry / persistent dismissals.
    QString promptId;

    QString title;
    QString message;

    // Free-form context for the UI (e.g. failedPoint, alarmCode,
    // toolNumber). Forwarded verbatim to the dialog and is also written
    // back into the suspended-parent's EntryContext::data under
    // "promptContext" on resume.
    QVariantMap context;

    QList<PromptChoice> choices;
};

Q_DECLARE_METATYPE(PromptSpec)
Q_DECLARE_METATYPE(PromptChoice)

#endif // USERPROMPTSPEC_H
