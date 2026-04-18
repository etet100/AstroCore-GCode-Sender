#ifndef ERRORBEHAVIOR_H
#define ERRORBEHAVIOR_H

#include "abstractstatebehavior.h"

class ErrorBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        explicit ErrorBehavior(QString error, QObject *parent = nullptr);
        ~ErrorBehavior();

        QString description() override;
        Type type() const override { return Type::Error; }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(AbstractStateBehavior *next) override;

    protected:
        QString name() const override { return "Error"; }

    private:
        QString m_error;
};

#endif // ERRORBEHAVIOR_H
