#include "errorbehavior.h"

ErrorBehavior::ErrorBehavior(QString error, QObject *parent) : StateBehavior(parent), m_error(error)
{
}

ErrorBehavior::~ErrorBehavior()
{
}

QString ErrorBehavior::description()
{
    return QString("Err: %1").arg(m_error);
}

StateBehavior::Result ErrorBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    return Result::Ok;
}

StateBehavior::Result ErrorBehavior::doOnExit(StateBehavior *next)
{
    return Result::Ok;
}
