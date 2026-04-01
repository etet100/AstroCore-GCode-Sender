#include "statebehaviormanager.h"
#include "core/state_behavior/statebehavior.h"
#include "communicator.h"
#include <QDebug>

StateBehaviorManager::StateBehaviorManager(QObject *signalEmitter)
    : QObject(nullptr),
      m_signalEmitter(signalEmitter)
{
}

void StateBehaviorManager::requestTransition(StateBehavior *nsb)
{
    if (nsb != nullptr) {
        qDebug() << "[Behavior][Manager] Transition requested to" << nsb->description();
    }
    m_nsb = nsb;
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

bool StateBehaviorManager::execute(StateBehavior *sb, bool force, CommunicatorApi *comApi)
{
    if (m_sb != nullptr) {
        if (!m_sb->onAboutToChange(sb, force)) {
            qDebug() << "[Behavior][Manager] Transition from" << m_sb->description()
                     << "to" << sb->description() << "is not allowed";
            return false;
        }

        if (m_sb->onExit(sb) == StateBehavior::Result::WaitForAsyncResult) {
            QObject::connect(m_sb, &StateBehavior::asyncCompleted, m_signalEmitter, [this, sb, comApi]() {
                qDebug() << "[Behavior][Manager] State behavior changed from"
                         << m_sb->description() << "to" << sb->description() << " (async exit)";
                this->finalizeExecute(sb, comApi);
            }, Qt::ConnectionType::SingleShotConnection);

            return true;
        }

        qDebug() << "[Behavior][Manager] State behavior changed from"
                 << m_sb->description() << "to" << sb->description();
    } else {
        qDebug() << "[Behavior][Manager] State behavior set to" << sb->description();
    }

    return finalizeExecute(sb, comApi);
}

bool StateBehaviorManager::finalizeExecute(StateBehavior *sb, CommunicatorApi *comApi)
{
    if (!sb->eventsAttached()) {
        Communicator *communicator = qobject_cast<Communicator*>(m_signalEmitter);
        if (communicator) {
            QObject::connect(sb, &StateBehavior::transition, communicator,
                           &Communicator::onStateRequestsTransition, Qt::ConnectionType::UniqueConnection);
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

    QPointer<StateBehavior> psb = m_sb;
    if (sb->onEntry(comApi, psb) == StateBehavior::Result::WaitForAsyncResult) {
        QObject::connect(sb, &StateBehavior::asyncCompleted, m_signalEmitter, [this, sb]() {
            qDebug() << "[Behavior][Manager] State behavior entry completed"
                     << sb->description() << " (async enter)";

            m_sb = sb;
            m_gc.track(sb);
            emit stateBehaviorChanged(sb);
        }, Qt::ConnectionType::SingleShotConnection);

        return true;
    }

    m_sb = sb;
    m_gc.track(sb);
    emit stateBehaviorChanged(sb);

    return true;
}

void StateBehaviorManager::dispose()
{
    m_gc.cleanup();
}
