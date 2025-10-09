// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcode.h"

GCode::GCode(QObject *parent) : QObject(parent) {
    reset();
}

// void GCode::setData(QList<GCodeItem> data)
// {
//     reset();
//     append(data);
// }

void GCode::reset(int commandIndex)
{
    m_commandIndex = commandIndex;
    m_processedCommandIndex = commandIndex;
}

void GCode::resetProcessed(int commandIndex)
{
    m_processedCommandIndex = commandIndex;
}

QString GCode::command()
{
    return this[m_commandIndex].command();
}

void GCode::advanceCommandIndex()
{
    m_commandIndex++;
}

bool GCode::isLastCommand()
{
    return m_commandIndex == count() - 1;
}

bool GCode::noMoreCommands()
{
    return m_commandIndex > count() - 1;
}

bool GCode::hasMoreCommands()
{
    return m_commandIndex <= count() - 1;
}

int GCode::lastCommandIndex()
{
    return count() - 1;
}

bool GCode::isLastCommandProcessed()
{
    return m_processedCommandIndex == count() - 1;
}

void GCode::commandSent()
{
    data()[m_commandIndex].state = GCodeItem::Sent;
    // m_currentModel->setData(m_currentModel->index(m_commandIndex, 2), GCodeItem::Sent);
    //        m_form->currentModel().setData(m_form->currentModel().index(m_streamer->commandIndex(), 2), GCodeItem::Sent);
}
