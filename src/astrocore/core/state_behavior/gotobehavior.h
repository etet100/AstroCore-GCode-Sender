// This file is a part of "G-Pilot GCode Sender" application.

#ifndef F7361A36_9D66_4126_9302_5C7751E1EAC2
#define F7361A36_9D66_4126_9302_5C7751E1EAC2

#ifndef GOTOBEHAVIOR_H
#define GOTOBEHAVIOR_H

#include "abstractstatebehavior.h"

class GoToBehavior : public AbstractStateBehavior
{
    public:
        explicit GoToBehavior(QPointF target, int feedRate,
                              bool delegateAlarmToParent = false,
                              QObject *parent = nullptr);
        QString description() override { return "Go to..."; }
        Type type() const override { return Type::GoTo; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Abort };
        }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(AbstractStateBehavior *next) override;
        void onAlarm(int code) override;

    protected:
        QString name() const override { return "GoTo"; }
        bool doAction(const Action &action) override;

    private:
        QPointF m_target;
        int m_feedRate;
        bool m_delegateAlarmToParent;
        std::optional<QCoro::Task<void>> m_goToTask;
        QCoro::Task<void> runGoToSequence();
        void stopJogging();

        // Routes an active alarm either to the parent (when delegateAlarmToParent)
        // or to a fresh AlarmBehavior.
        void emitAlarmExit();

        // Non-alarm failure exit: delegates to parent with a reason or transitions
        // to ErrorBehavior when not delegating.
        void emitFailureExit(const QString &reason);
};

#endif // GOTOBEHAVIOR_H


#endif /* F7361A36_9D66_4126_9302_5C7751E1EAC2 */
