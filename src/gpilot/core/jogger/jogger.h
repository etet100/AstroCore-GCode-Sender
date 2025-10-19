// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef JOGGER_H
#define JOGGER_H

#include <QObject>
#include "core/globals.h"

class Communicator;
class ConfigurationJogging;

class Jogger : public QObject
{
    Q_OBJECT

    public:
        Jogger(Communicator &communicator, const ConfigurationJogging &configuration);

        void jog(Axis axis, bool positiveDir);
        void jog(JoggindDir dir);
        void stop();

    private:
        Communicator &m_communicator;
        const ConfigurationJogging &m_configuration;
        void jogStart(QVector3D vector);
        void jogContinuous();
};


#endif // JOGGER_H
