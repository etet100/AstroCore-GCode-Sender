#ifndef STATEBEHAVIORMANAGER_H
#define STATEBEHAVIORMANAGER_H

#include <QPointer>
#include <QObject>
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
        bool hasPendingTransition() const { return m_nsb != nullptr; }
        bool hasCurrent() const { return m_sb != nullptr && !m_sb.isNull(); }

        void requestTransition(StateBehavior *nsb);
        bool processTransition();
        bool execute(StateBehavior *sb, bool force, CommunicatorApi *comApi);
        void dispose();
        bool finalizeExecute(StateBehavior *sb, CommunicatorApi *comApi);

    signals:
        void stateBehaviorChanged(StateBehavior *sb);

    private:
        QPointer<StateBehavior> m_sb = nullptr;
        QPointer<StateBehavior> m_nsb = nullptr;
        QObject *m_signalEmitter = nullptr;
        StateBehaviorGarbageCollector m_gc;
};

#endif // STATEBEHAVIORMANAGER_H
