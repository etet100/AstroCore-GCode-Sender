// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "jogger.h"
#include "core/communicator/communicator.h"
#include "core/scripting/scriptvars.h"

Jogger::Jogger(Communicator &communicator, const ConfigurationJogging &configuration) : m_communicator(communicator), m_configuration(configuration) {
}

void Jogger::jog(Axis axis, bool positiveDir)
{
    assert(&m_communicator.machineConfiguration());
    // m_communicator.jog(axis, positiveDir);

    QVector3D vector(0, 0, 0);
    switch (axis) {
        case Axis::X:
            vector.setX(positiveDir ? 1 : -1);
            break;
        case Axis::Y:
            vector.setY(positiveDir ? 1 : -1);
            break;
        case Axis::Z:
            vector.setZ(positiveDir ? 1 : -1);
            break;
        default:
            return;
    }

    bool unitsInches = m_communicator.machineConfiguration().unitsInches();
    vector *= m_configuration.step();
    qDebug() << "[Jogger]" << (int)axis << positiveDir << m_configuration.step();

    m_communicator.sendCommand(
        CommandSource::System,
        QString("$J=%5G91X%1Y%2Z%3F%4")
            .arg(vector.x(), 0, 'f', unitsInches ? 4 : 3)
            .arg(vector.y(), 0, 'f', unitsInches ? 4 : 3)
            .arg(vector.z(), 0, 'f', unitsInches ? 4 : 3)
            .arg(m_configuration.feed())
            .arg(unitsInches ? "G20" : "G21"),
        -3
    );
}

void Jogger::jog(JoggindDir dir)
{
    switch (dir) {
        case JoggindDir::XPlus:
            jog(Axis::X, true);
            break;
        case JoggindDir::XMinus:
            jog(Axis::X, false);
            break;
        case JoggindDir::YPlus:
            jog(Axis::Y, true);
            break;
        case JoggindDir::YMinus:
            jog(Axis::Y, false);
            break;
        case JoggindDir::ZPlus:
            jog(Axis::Z, true);
            break;
        case JoggindDir::ZMinus:
            jog(Axis::Z, false);
            break;
        default:
            break;
    }
}

void Jogger::stop()
{
    m_communicator.clearQueue();
    m_communicator.sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);
    // while (m_communicator->deviceState() == DeviceState::Jog) {
    //     qApp->processEvents();
    // }
}

void Jogger::jogStart(QVector3D vector)
{
    bool unitsInches = m_communicator.machineConfiguration().unitsInches();

    // Bounds
    QVector3D b = m_communicator.machineConfiguration().machineBounds();
    // Current machine coords
    // @TODO use m_communicator storedVars
    PositionTracker* tracker = m_communicator.positionTracker();
    ScriptVars& vars = tracker->scriptVars();
    QVector3D m(
        m_communicator.toMetric(vars.Mx()),
        m_communicator.toMetric(vars.My()),
        m_communicator.toMetric(vars.Mz())
    );
    // Distance to bounds
    QVector3D t;
    // Minimum distance to bounds
    double d = 0;
    if (m_communicator.machineConfiguration().softLimitsEnabled()) {
        t = QVector3D(vector.x() * b.x() < 0 ? 0 - m.x() : b.x() - m.x(),
                      vector.y() * b.y() < 0 ? 0 - m.y() : b.y() - m.y(),
                      vector.z() * b.z() < 0 ? 0 - m.z() : b.z() - m.z());
        for (int i = 0; i < 3; i++) if ((vector[i] && (qAbs(t[i]) < d)) || (vector[i] && !d)) d = qAbs(t[i]);
        // Coords not aligned, add some bounds offset
        d -= unitsInches ? m_communicator.toMetric(0.0005) : 0.005;
    } else {
        for (int i = 0; i < 3; i++) if (vector[i] && (qAbs(b[i]) > d)) d = qAbs(b[i]);
    }

    // Jog vector
    QVector3D vec = vector * m_communicator.toInches(d);

    if (vec.length()) {
        m_communicator.sendCommand(CommandSource::System, QString("$J=%5G91X%1Y%2Z%3F%4")
                                        .arg(vec.x(), 0, 'f', unitsInches ? 4 : 3)
                                        .arg(vec.y(), 0, 'f', unitsInches ? 4 : 3)
                                        .arg(vec.z(), 0, 'f', unitsInches ? 4 : 3)
                                        .arg(m_configuration.feed())
                                        .arg(unitsInches ? "G20" : "G21")
                                        , -2);
    }
}

void Jogger::jogContinuous()
{
//     static bool block = false;
//     static QVector3D lastVector(0, 0, 0);

//     if ((ui->jog->isContinuous()) && !block) {
//         if (ui->jog->jogVector() != lastVector) {
//             // Store jog vector before block
//             QVector3D vector = ui->jog->jogVector();

//             // Stop jogging
//             if (lastVector.length()) {
//                 lastVector = vector;
//                 block = true;

//                 m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);

//                 if (!vector.length()) {
//                     return;
//                 }

//                 QObject *obj = new QObject(this);
//                 connect(m_communicator, &Communicator::deviceStateChanged, obj, [this, obj, vector] (DeviceState state) {
//                     qDebug() << "deviceStateChanged" << (int) state;
//                     if (state != DeviceState::Jog) {
//                         jogStart(vector);
//                         obj->deleteLater();
//                     }
//                 });

//                 block = false;
//             } else {
//                 lastVector = vector;
//                 jogStart(vector);
//             }
//         }
//     }
}
