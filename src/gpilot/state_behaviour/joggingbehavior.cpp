// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "../globals.h"
#include "joggingbehavior.h"
#include "../communicator.h"
#include "idlebehavior.h"
#include "pausebehavior.h"
#include "alarmbehavior.h"

JoggingBehavior::JoggingBehavior(StateBehavior *previous, QObject *parent)
    : StateBehavior{previous, parent}
    , m_currentDirection(JoggindDir::None)
    , m_feedRate(100)
    , m_distance(0)
    , m_isJogging(false)
{
}

void JoggingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);

    // Initialize jogging state
    m_isJogging = false;
}

void JoggingBehavior::onExit()
{
    // Stop jogging before exiting
    if (m_isJogging) {
        stopJogging();
    }
}

void JoggingBehavior::onDeviceStateChanged(DeviceState state)
{
    if (state == DeviceState::Idle) {
        // Return to idle if not jogging
        if (!m_isJogging) {
            emit transition(this, new IdleBehavior(this));
        }
    } else if (state == DeviceState::Alarm) {
        // Stop jogging and handle alarm
        stopJogging();
        emit transition(this, new AlarmBehavior(this));
    } else if (state == DeviceState::Hold0 || state == DeviceState::Hold1) {
        // Machine hold - go to pause state
        emit transition(this, new PauseBehavior(this, PauseBehavior::PauseSource::Jogging));
    }
}

void JoggingBehavior::onCommandResponse(QString command, QStringList response)
{
    // Obsługa odpowiedzi na komendy joggingu
    if (command.startsWith("$J=") && !response.contains("error")) {
        // Komenda joggingu została zaakceptowana
        m_isJogging = true;
    } else if (command == QString(QChar(GRBL_LIVE_JOG_CANCEL))) {
        // Komenda zatrzymania joggingu
        m_isJogging = false;
    }

    // Sprawdzenie, czy nie wystąpił błąd
    if (response.contains("error")) {
        m_isJogging = false;
        // Można tutaj obsłużyć różne rodzaje błędów
    }
}

void JoggingBehavior::startJogging(JoggindDir direction, double feedRate, double distance)
{
    if (!m_communicator) {
        return;
    }

    m_currentDirection = direction;
    m_feedRate = feedRate;
    m_distance = distance;

    // Tworzenie komendy joggingu w zależności od kierunku
    QString jogCommand = "$J=";

    switch (direction) {
        case JoggindDir::XPlus:
            jogCommand += "G91 G21 X" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::XMinus:
            jogCommand += "G91 G21 X-" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::YPlus:
            jogCommand += "G91 G21 Y" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::YMinus:
            jogCommand += "G91 G21 Y-" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::ZPlus:
            jogCommand += "G91 G21 Z" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::ZMinus:
            jogCommand += "G91 G21 Z-" + QString::number(distance > 0 ? distance : 100);
            break;
        default:
            return; // Nieznany kierunek
    }

    // Dodaj prędkość posuwu
    jogCommand += " F" + QString::number(feedRate);

    // Wysłanie komendy joggingu
    m_communicator->sendCommand(CommandSource::GeneralUI, jogCommand, TABLE_INDEX_UI);
}

void JoggingBehavior::stopJogging()
{
    if (!m_communicator || !m_isJogging) {
        return;
    }

    // Wysłanie komendy zatrzymania joggingu
    m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);
    m_isJogging = false;
    m_currentDirection = JoggindDir::None;
}

void JoggingBehavior::setJoggingFeedRate(double feedRate)
{
    m_feedRate = feedRate;

    // Jeśli jesteśmy w trakcie joggingu, możemy chcieć zaktualizować prędkość
    // Jednak w większości kontrolerów, aby zmienić prędkość joggingu,
    // trzeba zatrzymać aktualny jogging i rozpocząć nowy z nową prędkością
    if (m_isJogging) {
        JoggindDir currentDir = m_currentDirection;
        stopJogging();
        startJogging(currentDir, feedRate, m_distance);
    }
}
