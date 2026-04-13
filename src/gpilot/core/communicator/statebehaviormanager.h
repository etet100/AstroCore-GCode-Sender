#ifndef STATEBEHAVIORMANAGER_H
#define STATEBEHAVIORMANAGER_H

#include <QPointer>
#include <QObject>
#include <QList>
#include "statebehaviorgarbagecollector.h"
#include "core/state_behavior/statebehavior.h"

class CommunicatorApi;

class StateBehaviorManager : public QObject
{
    Q_OBJECT

    public:
        explicit StateBehaviorManager(QObject *signalEmitter);

        StateBehavior* current() const { return m_sb.data(); }
        StateBehavior* next() const { return m_nsb.data(); }
        StateBehavior::TransitionKind pendingTransitionKind() const { return m_nsbKind; }
        bool hasPendingTransition() const { return m_nsb != nullptr; }
        bool hasPendingResume() const { return m_resumeRequested; }
        bool hasCurrent() const { return m_sb != nullptr && !m_sb.isNull(); }

        void requestTransition(StateBehavior *nsb,
                               StateBehavior::TransitionKind kind = StateBehavior::TransitionKind::Replace);
        void requestResume();
        void clearPendingResume() { m_resumeRequested = false; }
        bool processTransition();
        bool execute(StateBehavior *sb, bool force, CommunicatorApi *comApi,
                     StateBehavior::TransitionKind kind = StateBehavior::TransitionKind::Replace);
        bool resumePrevious(CommunicatorApi *comApi);
        void dispose();
        bool finalizeExecute(StateBehavior *sb, CommunicatorApi *comApi,
                             StateBehavior::EntryContext ctx);

    signals:
        void stateBehaviorChanged(StateBehavior *sb);

    private:
        QPointer<StateBehavior> m_sb = nullptr;
        QPointer<StateBehavior> m_nsb = nullptr;
        StateBehavior::TransitionKind m_nsbKind = StateBehavior::TransitionKind::Replace;
        bool m_resumeRequested = false;
        QList<QPointer<StateBehavior>> m_suspended;  // stack: back() is top
        QObject *m_signalEmitter = nullptr;
        StateBehaviorGarbageCollector m_gc;

        void flushSuspendedToGC();
        StateBehavior::EntryContext makeEntryContext(StateBehavior *from) const;
};

#endif // STATEBEHAVIORMANAGER_H
