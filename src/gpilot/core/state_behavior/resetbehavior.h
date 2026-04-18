// This file is a part of "G-Pilot GCode Sender" application.

#ifndef RESETBEHAVIOR_H
#define RESETBEHAVIOR_H

#include "abstractstatebehavior.h"

class ResetBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        explicit ResetBehavior(QObject *parent = nullptr);
        QString description() override { return "Reset"; }
        Type type() const override { return Type::Reset; }
        Result onRawResponse(QString response) override;
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        void onAlarm(int code) override;

    protected:
        QString name() const override { return "Reset"; }

    private:
        enum Stage {
            None,
            SentReset,
            Completed
        };
        Stage m_stage = None;

        bool dataIsReset(QString data);
};

#endif // RESETBEHAVIOR_H
