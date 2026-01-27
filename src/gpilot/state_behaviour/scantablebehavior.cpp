#include "scantablebehavior.h"

ScanTableBehavior::ScanTableBehavior(QObject* parent)
    : StateBehavior{parent}
{}

StateBehavior::Result ScanTableBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    return StateBehavior::Result::Ok;
}
