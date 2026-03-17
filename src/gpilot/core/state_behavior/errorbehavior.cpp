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

StateBehavior::Result ErrorBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    return Result::Ok;
}

StateBehavior::Result ErrorBehavior::onExit(StateBehavior *next)
{
    return Result::Ok;
}
