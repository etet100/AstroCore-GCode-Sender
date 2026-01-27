#ifndef SCANTABLEBEHAVIOR_H
#define SCANTABLEBEHAVIOR_H

#include "statebehavior.h"

class ScanTableBehavior : public StateBehavior
{
    public:
        explicit ScanTableBehavior(QObject* parent = nullptr);
        QString description() override { return "Scanning table"; }
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous);

    protected:
        QString name() const override { return "ScanTableBehavior"; }
};

#endif // SCANTABLEBEHAVIOR_H
