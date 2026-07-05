#ifndef STATEBEHAVIORGARBAGECOLLECTOR_H
#define STATEBEHAVIORGARBAGECOLLECTOR_H

#include <QList>
#include <QPointer>

class AbstractStateBehavior;

class StateBehaviorGarbageCollector
{
public:
    StateBehaviorGarbageCollector(int maxRetainedObjects = 3);

    void track(AbstractStateBehavior *sb);
    void cleanup();
    int count() const { return m_trackedBehaviors.count(); }

private:
    QList<QPointer<AbstractStateBehavior>> m_trackedBehaviors;
    int m_maxRetainedObjects;
};

#endif // STATEBEHAVIORGARBAGECOLLECTOR_H
