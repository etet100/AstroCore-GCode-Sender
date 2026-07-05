#ifndef STATEBEHAVIORMANAGER_H
#define STATEBEHAVIORMANAGER_H

#include <QPointer>
#include <QObject>
#include <QList>
#include "statebehaviorgarbagecollector.h"
#include "core/state_behavior/abstractstatebehavior.h"

class CommunicatorApi;

class StateBehaviorManager : public QObject
{
    Q_OBJECT

    public:
        explicit StateBehaviorManager(QObject *signalEmitter);

        AbstractStateBehavior* current() const { return m_sb.data(); }
        AbstractStateBehavior* next() const { return m_nsb.data(); }
        AbstractStateBehavior::TransitionKind pendingTransitionKind() const { return m_nsbKind; }
        bool hasPendingTransition() const { return m_nsb != nullptr; }
        bool hasPendingResume() const { return m_resumeRequested; }
        bool hasCurrent() const { return m_sb != nullptr && !m_sb.isNull(); }

        void requestTransition(AbstractStateBehavior *nsb,
                               AbstractStateBehavior::TransitionKind kind = AbstractStateBehavior::TransitionKind::Replace);
        void requestResume();
        void clearPendingResume() { m_resumeRequested = false; }
        bool processTransition();
        bool execute(AbstractStateBehavior *sb, bool force, CommunicatorApi *comApi,
                     AbstractStateBehavior::TransitionKind kind = AbstractStateBehavior::TransitionKind::Replace);
        bool resumePrevious(CommunicatorApi *comApi);
        void dispose();
        bool finalizeExecute(AbstractStateBehavior *sb, CommunicatorApi *comApi,
                             AbstractStateBehavior::EntryContext ctx);

    signals:
        void stateBehaviorChanged(AbstractStateBehavior *sb);

    private:
        QPointer<AbstractStateBehavior> m_sb = nullptr;
        QPointer<AbstractStateBehavior> m_nsb = nullptr;
        AbstractStateBehavior::TransitionKind m_nsbKind = AbstractStateBehavior::TransitionKind::Replace;
        bool m_resumeRequested = false;
        QList<QPointer<AbstractStateBehavior>> m_suspended;  // stack: back() is top
        QObject *m_signalEmitter = nullptr;
        StateBehaviorGarbageCollector m_gc;

        void flushSuspendedToGC();
        AbstractStateBehavior::EntryContext makeEntryContext(AbstractStateBehavior *from) const;
};

#endif // STATEBEHAVIORMANAGER_H
