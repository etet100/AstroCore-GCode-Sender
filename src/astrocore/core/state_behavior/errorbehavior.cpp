#include "errorbehavior.h"

ErrorBehavior::ErrorBehavior(QString error, QObject *parent) : AbstractStateBehavior(parent), m_error(error)
{
}

ErrorBehavior::~ErrorBehavior()
{
}

QString ErrorBehavior::description()
{
    return QString("Err: %1").arg(m_error);
}

AbstractStateBehavior::Result ErrorBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    return Result::Ok;
}

AbstractStateBehavior::Result ErrorBehavior::doOnExit(AbstractStateBehavior *next)
{
    return Result::Ok;
}
