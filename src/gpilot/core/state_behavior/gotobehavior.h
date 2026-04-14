// This file is a part of "G-Pilot GCode Sender" application.

#ifndef F7361A36_9D66_4126_9302_5C7751E1EAC2
#define F7361A36_9D66_4126_9302_5C7751E1EAC2

#ifndef GOTOBEHAVIOR_H
#define GOTOBEHAVIOR_H

#include "statebehavior.h"

class GoToBehavior : public StateBehavior
{
    public:
        explicit GoToBehavior(QPointF target, int feedRate, QObject *parent = nullptr);
        QString description() override { return "Go to..."; }
        Type type() const override { return Type::GoTo; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Abort };
        }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(StateBehavior *next) override;
        void onAlarm(int code) override;

    protected:
        QString name() const override { return "GoTo"; }
        bool doAction(const Action &action) override;

    private:
        QPointF m_target;
        int m_feedRate;
        std::optional<QCoro::Task<void>> m_goToTask;
        QCoro::Task<void> runGoToSequence();
        void stopJogging();
};

#endif // GOTOBEHAVIOR_H


#endif /* F7361A36_9D66_4126_9302_5C7751E1EAC2 */
