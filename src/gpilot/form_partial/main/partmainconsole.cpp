// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "partmainconsole.h"
#include "ui_partmainconsole.h"
#include <QScrollBar>
#include <QCompleter>

partMainConsole::partMainConsole(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainConsole)
{
    ui->setupUi(this);

    #ifndef UNIX
        ui->cboCommand->setStyleSheet("QComboBox {padding: 2;} QComboBox::drop-down {width: 0; border-style: none;} QComboBox::down-arrow {image: url(noimg);	border-width: 0;}");
    #endif

    ui->cboCommand->setMinimumHeight(ui->cboCommand->height());
    ui->cmdClearConsole->setFixedHeight(ui->cboCommand->height());
    ui->cmdCommandSend->setFixedHeight(ui->cboCommand->height());
}

void partMainConsole::initialize(ConfigurationConsole &configurationConsole)
{
    m_configurationConsole = &configurationConsole;

    if (m_configurationConsole->commandAutoCompletion()) {
        QCompleter completer = ui->cboCommand->completer();
        completer.setCompletionMode(QCompleter::InlineCompletion);
    } else {
        ui->cboCommand->setCompleter(nullptr);
    }

    //m_completerModel.setCommands(m_configurationConsole->commandHistory());
    ui->cboCommand->addItems(m_configurationConsole->commandHistory());
    ui->cboCommand->setCurrentIndex(-1);

    if (m_configurationConsole->darkBackgroundMode()) {
        applyDarkBackgroundMode();
    }
}

partMainConsole::~partMainConsole()
{
    delete ui;
}

void partMainConsole::applyDarkBackgroundMode()
{
    ui->txtConsole->setStyleSheet("QPlainTextEdit { background-color: #000000; color: #FFFFFF; }");
}

void partMainConsole::append(QString text)
{
    ui->txtConsole->appendPlainText(text);
}

void partMainConsole::appendSystem(QString text)
{
    if (!m_configurationConsole->showSystemCommands()) return;
    append(text);
}

void partMainConsole::append(CommandAttributes commandAttributes)
{
    append(">> " + commandAttributes.commandLine);

    QTextBlock block = lastBlock();
    BlockData *blockData = new CommandBlockData(block.blockNumber(), commandAttributes.commandIndex);
    block.setUserData(blockData);
}

void partMainConsole::appendFiltered(CommandAttributes commandAttributes)
{
    switch (commandAttributes.source) {
        case CommandSource::System:
            if (!m_configurationConsole->showSystemCommands()) return;
            break;
        case CommandSource::Program:
        case CommandSource::ProgramAdditionalCommands:
            if (!m_configurationConsole->showProgramCommands()) return;
            break;
        case CommandSource::Console:
        case CommandSource::GeneralUI:
            if (!m_configurationConsole->showUiCommands()) return;
            break;
    }

    append(commandAttributes);
}

void partMainConsole::appendResponse(CommandAttributes commandAttributes)
{
    QTextDocument *document = ui->txtConsole->document();
    QTextBlock block = document->lastBlock();
    int blocksCounter = 0;
    do {
        BlockData *unknownBlockData = static_cast<BlockData *>(block.userData());
        if (unknownBlockData && unknownBlockData->type() == ItemType::Command) {
            CommandBlockData *blockData = static_cast<CommandBlockData *>(unknownBlockData);
            if (blockData->commandIndex() == commandAttributes.commandIndex) {
                QTextCursor cursor(block);

                cursor.beginEditBlock();
                cursor.movePosition(QTextCursor::EndOfBlock);
                // @TODO response was added as multiple lines, do we have to do this?
                // cursor.insertText(" < " + QString(response).replace("; ", "\r\n"));
                cursor.insertHtml(QString("&nbsp;<span style='color: gray; font-size: 90%'>(%1)</span> ").arg(commandAttributes.response));
                cursor.endEditBlock();

                // do we need it anymore?
                block.setUserData(nullptr);

                return;
            }
        }
        // let's check max 50 blocks from the end
        if (blocksCounter++ > MAX_BLOCKS_TO_CHECK) {
            return;
        }

        block = block.previous();
    } while (block.isValid());
}

int partMainConsole::appendProgress(QString text)
{
    ProgressBlockData *blockData = new ProgressBlockData(text, m_index++);
    append(blockData->text(0));

    QTextBlock block = lastBlock();
    block.setUserData(blockData);

    return blockData->index();
}

void partMainConsole::setProgress(int index, int progress)
{
    QTextDocument *document = ui->txtConsole->document();
    QTextBlock block = document->lastBlock();
    int blocksCounter = 0;
    do {
        BlockData *unknownBlockData = static_cast<BlockData *>(block.userData());
        if (unknownBlockData && unknownBlockData->type() == ItemType::Progress) {
            ProgressBlockData *blockData = static_cast<ProgressBlockData *>(unknownBlockData);
            if (blockData->index() == index) {
                QTextCursor cursor(block);

                cursor.beginEditBlock();
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.select(QTextCursor::LineUnderCursor);
                cursor.insertText(blockData->text(progress));
                cursor.endEditBlock();

                return;
            }
        }
        // let's check max 50 blocks from the end
        if (blocksCounter++ > MAX_BLOCKS_TO_CHECK) {
            return;
        }

        block = block.previous();
    } while (block.isValid());
}

void partMainConsole::clear()
{
    ui->txtConsole->clear();
}

void partMainConsole::send()
{
    QString command = ui->cboCommand->currentText().trimmed();
    ui->cboCommand->clearEditText();

    if (command.isEmpty()) return;

    m_configurationConsole->setCommandHistory(ui->cboCommand->items());

    emit newCommand(command);
}

bool partMainConsole::isScrolledToEnd()
{
    return ui->txtConsole->verticalScrollBar()->value()
                        == ui->txtConsole->verticalScrollBar()->maximum();
}

void partMainConsole::scrollToEnd()
{
    ui->txtConsole->verticalScrollBar()->setValue(
        ui->txtConsole->verticalScrollBar()->maximum());
}

QTextBlock partMainConsole::lastBlock()
{
    return ui->txtConsole->document()->lastBlock();
}

void partMainConsole::onClearClicked()
{
    clear();
    emit consoleCleared();
}

void partMainConsole::onSendClicked()
{
    send();
}

QString partMainConsole::ProgressBlockData::text(int progress)
{
    return QString("%1 [%2%]").arg(m_text).arg(progress);

    // ##### mode
    // // 100 -> 20, progress = x
    // int scaled = progress / 5;
    // int rest = 20 - scaled;

    // return QString("%1 [%2%3]").arg(m_text).arg(QString("#").repeated(scaled)).arg(QString("_").repeated(rest));
}
