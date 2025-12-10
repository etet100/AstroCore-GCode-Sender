#ifndef ERRORBEHAVIOUR_H
#define ERRORBEHAVIOUR_H

#include "statebehavior.h"

class ErrorBehaviour : public StateBehavior
{
    Q_OBJECT

    public:
        explicit ErrorBehaviour(int code, QObject *parent = nullptr);
        ~ErrorBehaviour();

        QString description() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

    protected:
        QString name() const override { return "ErrorBehaviour"; }

    private:
        int m_errorCode;
};

#endif // ERRORBEHAVIOUR_H
