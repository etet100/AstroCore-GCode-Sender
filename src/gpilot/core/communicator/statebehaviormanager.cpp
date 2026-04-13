#include "statebehaviormanager.h"
#include "core/state_behavior/statebehavior.h"
#include "communicator.h"
#include <QDebug>

StateBehaviorManager::StateBehaviorManager(QObject *signalEmitter)
    : QObject(nullptr),
      m_signalEmitter(signalEmitter)
{
}

void StateBehaviorManager::requestTransition(StateBehavior *nsb, StateBehavior::TransitionKind kind)
{
    if (nsb != nullptr) {
        qDebug() << "[Behavior][Manager] Transition requested to" << nsb->description()
                 << (kind == StateBehavior::TransitionKind::Suspend ? "(suspend)" : "(replace)");
    }
    m_nsb = nsb;
    m_nsbKind = kind;
}

void StateBehaviorManager::requestResume()
{
    qDebug() << "[Behavior][Manager] Resume requested";
    m_resumeRequested = true;
}

bool StateBehaviorManager::processTransition()
{
    if (m_nsb == nullptr) {
        return false;
    }

    assert(!m_sb.isNull());

    StateBehavior *nsb = m_nsb;
    m_nsb = nullptr;

    qDebug() << "[Behavior][Manager] Processing transition to" << nsb->description();

    return true;
}

StateBehavior::EntryContext StateBehaviorManager::makeEntryContext(StateBehavior *from) const
{
    StateBehavior::EntryContext ctx;
    if (from) {
        ctx.previousType = from->type();
        ctx.data = from->exitData();
    }

    return ctx;
}

void StateBehaviorManager::flushSuspendedToGC()
{
    for (auto &ptr : m_suspended) {
        if (!ptr.isNull()) {
            qDebug() << "[Behavior][Manager] Flushing suspended" << ptr->description() << "to GC";
            m_gc.track(ptr.data());
        }
    }
    m_suspended.clear();
}

bool StateBehaviorManager::execute(StateBehavior *sb, bool force, CommunicatorApi *comApi,
                                    StateBehavior::TransitionKind kind)
{
    StateBehavior::EntryContext ctx = makeEntryContext(m_sb.data());

    if (m_sb != nullptr) {
        if (!m_sb->onAboutToChange(sb, force)) {
            qDebug() << "[Behavior][Manager] Transition from" << m_sb->description()
                     << "to" << sb->description() << "is not allowed";
            return false;
        }

        if (m_sb->onExit(sb) == StateBehavior::Result::WaitForAsyncResult) {
            QObject::connect(m_sb, &StateBehavior::asyncCompleted, m_signalEmitter, [this, sb, comApi, kind, ctx]() {
                qDebug() << "[Behavior][Manager] State behavior changed from"
                         << m_sb->description() << "to" << sb->description() << " (async exit)";

                if (kind == StateBehavior::TransitionKind::Suspend) {
                    m_suspended.append(m_sb);
                } else {
                    flushSuspendedToGC();
                    m_gc.track(m_sb.data());
                }

                this->finalizeExecute(sb, comApi, ctx);
            }, Qt::ConnectionType::SingleShotConnection);

            return true;
        }

        qDebug() << "[Behavior][Manager] State behavior changed from"
                 << m_sb->description() << "to" << sb->description()
                 << (kind == StateBehavior::TransitionKind::Suspend ? "(suspend)" : "(replace)");

        if (kind == StateBehavior::TransitionKind::Suspend) {
            m_suspended.append(m_sb);
        } else {
            flushSuspendedToGC();
            m_gc.track(m_sb.data());
        }
    } else {
        qDebug() << "[Behavior][Manager] State behavior set to" << sb->description();
    }

    return finalizeExecute(sb, comApi, ctx);
}

bool StateBehaviorManager::resumePrevious(CommunicatorApi *comApi)
{
    if (m_suspended.isEmpty()) {
        qDebug() << "[Behavior][Manager] resumePrevious requested but suspended stack is empty";
        return false;
    }

    // Drop any dead entries from the top.
    while (!m_suspended.isEmpty() && m_suspended.last().isNull()) {
        m_suspended.removeLast();
    }
    if (m_suspended.isEmpty()) {
        qDebug() << "[Behavior][Manager] resumePrevious: stack contained only dead entries";
        return false;
    }

    StateBehavior *resumed = m_suspended.takeLast().data();
    StateBehavior::EntryContext ctx = makeEntryContext(m_sb.data());

    if (m_sb != nullptr) {
        if (m_sb->onExit(resumed) == StateBehavior::Result::WaitForAsyncResult) {
            QObject::connect(m_sb, &StateBehavior::asyncCompleted, m_signalEmitter, [this, resumed, comApi, ctx]() {
                qDebug() << "[Behavior][Manager] Resumed" << resumed->description()
                         << "from" << m_sb->description() << "(async exit)";
                m_gc.track(m_sb.data());
                this->finalizeExecute(resumed, comApi, ctx);
            }, Qt::ConnectionType::SingleShotConnection);

            return true;
        }

        qDebug() << "[Behavior][Manager] Resumed" << resumed->description()
                 << "from" << m_sb->description();
        m_gc.track(m_sb.data());
    }

    return finalizeExecute(resumed, comApi, ctx);
}

bool StateBehaviorManager::finalizeExecute(StateBehavior *sb, CommunicatorApi *comApi,
                                            StateBehavior::EntryContext ctx)
{
    if (!sb->eventsAttached()) {
        Communicator *communicator = qobject_cast<Communicator*>(m_signalEmitter);
        if (communicator) {
            QObject::connect(sb, &StateBehavior::transition, communicator,
                           &Communicator::onStateRequestsTransition, Qt::ConnectionType::UniqueConnection);
            QObject::connect(sb, &StateBehavior::resumePrevious, communicator,
                           &Communicator::onStateRequestsResume, Qt::ConnectionType::UniqueConnection);
            QObject::connect(sb, &StateBehavior::error, communicator,
                           &Communicator::onStateError, Qt::ConnectionType::UniqueConnection);
            QObject::connect(sb, &StateBehavior::logSignal, communicator,
                           &Communicator::log, Qt::ConnectionType::UniqueConnection);
            QObject::connect(sb, &QObject::destroyed, m_signalEmitter, []() {
                qDebug() << "[Behavior][Manager] State behavior destroyed";
            });
        }

        sb->markEventsAttached();
    }

    if (sb->onEntry(comApi, ctx) == StateBehavior::Result::WaitForAsyncResult) {
        QObject::connect(sb, &StateBehavior::asyncCompleted, m_signalEmitter, [this, sb]() {
            qDebug() << "[Behavior][Manager] State behavior entry completed"
                     << sb->description() << " (async enter)";

            m_sb = sb;
            emit stateBehaviorChanged(sb);
        }, Qt::ConnectionType::SingleShotConnection);

        return true;
    }

    m_sb = sb;
    emit stateBehaviorChanged(sb);

    return true;
}

void StateBehaviorManager::dispose()
{
    m_gc.cleanup();
}
