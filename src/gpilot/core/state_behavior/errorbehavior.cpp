#include "errorbehavior.h"

ErrorBehavior::ErrorBehavior(int code, QObject *parent) : StateBehavior(parent), m_errorCode(code)
{
}

ErrorBehavior::~ErrorBehavior()
{
}

QString ErrorBehavior::description()
{
    return QString("Error: %1").arg(m_errorCode);
}

StateBehavior::Result ErrorBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    return Result::Ok;
}

StateBehavior::Result ErrorBehavior::onExit(StateBehavior *next)
{
    return Result::Ok;
}
