#ifndef ERRORBEHAVIOR_H
#define ERRORBEHAVIOR_H

#include "statebehavior.h"

class ErrorBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit ErrorBehavior(QString error, QObject *parent = nullptr);
        ~ErrorBehavior();

        QString description() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

    protected:
        QString name() const override { return "Error"; }

    private:
        QString m_error;
};

#endif // ERRORBEHAVIOR_H
