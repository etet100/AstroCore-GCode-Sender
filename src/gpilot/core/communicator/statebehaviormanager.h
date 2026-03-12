#ifndef STATEBEHAVIORMANAGER_H
#define STATEBEHAVIORMANAGER_H

#include <QPointer>
#include <QObject>
#include "statebehaviorgarbagecollector.h"

class StateBehavior;
class CommunicatorApi;

class StateBehaviorManager
{
public:
    StateBehaviorManager(QObject *signalEmitter);

    StateBehavior* current() const { return m_sb.data(); }
    StateBehavior* next() const { return m_nsb.data(); }
    bool hasPendingTransition() const { return m_nsb != nullptr; }
    bool hasCurrent() const { return m_sb != nullptr && !m_sb.isNull(); }

    void requestTransition(StateBehavior *nsb);
    bool processTransition();
    bool execute(StateBehavior *sb, bool force, CommunicatorApi *comApi);
    void dispose();
    bool finalizeExecute(StateBehavior *sb, CommunicatorApi *comApi);

private:
    QPointer<StateBehavior> m_sb = nullptr;
    QPointer<StateBehavior> m_nsb = nullptr;
    QObject *m_signalEmitter = nullptr;
    StateBehaviorGarbageCollector m_gc;
};

#endif // STATEBEHAVIORMANAGER_H
