#ifndef STATEBEHAVIORGARBAGECOLLECTOR_H
#define STATEBEHAVIORGARBAGECOLLECTOR_H

#include <QList>
#include <QPointer>

class StateBehavior;

class StateBehaviorGarbageCollector
{
public:
    StateBehaviorGarbageCollector(int maxRetainedObjects = 3);

    void track(StateBehavior *sb);
    void cleanup();
    int count() const { return m_trackedBehaviors.count(); }

private:
    QList<QPointer<StateBehavior>> m_trackedBehaviors;
    int m_maxRetainedObjects;
};

#endif // STATEBEHAVIORGARBAGECOLLECTOR_H
