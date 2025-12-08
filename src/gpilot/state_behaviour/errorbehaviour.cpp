#include "errorbehaviour.h"

ErrorBehaviour::ErrorBehaviour(int code, QObject *parent) : StateBehavior(parent), m_errorCode(code)
{
}

ErrorBehaviour::~ErrorBehaviour()
{
}

QString ErrorBehaviour::name()
{
    return QString("Error: %1").arg(m_errorCode);
}

StateBehavior::Result ErrorBehaviour::onEntry(Communicator *communicator, StateBehavior *previous)
{
    return Result::Ok;
}

StateBehavior::Result ErrorBehaviour::onExit(StateBehavior *next)
{
    return Result::Ok;
}
