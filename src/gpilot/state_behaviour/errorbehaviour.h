#ifndef ERRORBEHAVIOUR_H
#define ERRORBEHAVIOUR_H

#include "statebehavior.h"

class ErrorBehaviour : public StateBehavior
{
    Q_OBJECT

    public:
        explicit ErrorBehaviour(int code, QObject *parent = nullptr);
        ~ErrorBehaviour();

        QString name() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

    private:
        int m_errorCode;
};

#endif // ERRORBEHAVIOUR_H
