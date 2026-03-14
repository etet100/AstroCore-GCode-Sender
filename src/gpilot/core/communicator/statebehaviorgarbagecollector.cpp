#include "statebehaviorgarbagecollector.h"
#include "core/state_behavior/statebehavior.h"
#include <QDebug>

StateBehaviorGarbageCollector::StateBehaviorGarbageCollector(int maxRetainedObjects)
    : m_maxRetainedObjects(maxRetainedObjects)
{
}

void StateBehaviorGarbageCollector::track(StateBehavior *sb)
{
    if (sb == nullptr) {
        return;
    }

    for (int i = 0; i < m_trackedBehaviors.size(); ++i) {
        if (m_trackedBehaviors[i] == sb) {
            m_trackedBehaviors.move(i, 0);
            qDebug() << "[StateBehavior][GC] Moved" << sb->description() << "to front, total tracked:" << m_trackedBehaviors.count();
            return;
        }
    }

    m_trackedBehaviors.prepend(QPointer<StateBehavior>(sb));
    qDebug() << "[StateBehavior][GC] Tracking new behavior" << sb->description() << ", total tracked:" << m_trackedBehaviors.count();
}

void StateBehaviorGarbageCollector::cleanup()
{
    int initialCount = m_trackedBehaviors.count();

    m_trackedBehaviors.erase(
        std::remove_if(m_trackedBehaviors.begin(), m_trackedBehaviors.end(),
            [](const QPointer<StateBehavior> &ptr) { return ptr.isNull(); }),
        m_trackedBehaviors.end()
    );

    while (m_trackedBehaviors.size() > m_maxRetainedObjects) {
        QPointer<StateBehavior> sb = m_trackedBehaviors.takeLast();
        if (!sb.isNull()) {
            qDebug() << "[StateBehavior][GC] Deleting" << sb->description();
            delete sb.data();
        }
    }

    int deleted = initialCount - m_trackedBehaviors.count();
    if (deleted > 0) {
        qDebug() << "[StateBehavior][GC] Cleanup complete: deleted" << deleted << "behaviors, retained" << m_trackedBehaviors.count();
    }
}
