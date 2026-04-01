// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "partmainconsole.h"
#include "ui_partmainconsole.h"
#include "ui/utils/thememanager.h"
#include "utils/utils.h"
#include <QScrollBar>
#include <QCompleter>
#include <QKeyEvent>
#include <QLineEdit>

PartMainConsole::PartMainConsole(QWidget *parent)
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

    ui->cboCommand->installEventFilter(this);

    static bool dark = ThemeManager::instance().dark();
    if (dark) {
        Utils::invertButtonIconColors({
            ui->cmdClearConsole,
            ui->cmdCommandSend,
        });
    }
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool dark_) {
        if (dark != dark_) {
            dark = dark_;
            Utils::invertButtonIconColors({
                ui->cmdClearConsole,
                ui->cmdCommandSend,
            });
        }
    });
}

void PartMainConsole::initialize(ConfigurationConsole &configurationConsole)
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

PartMainConsole::~PartMainConsole()
{
    delete ui;
}

void PartMainConsole::applyDarkBackgroundMode()
{
    ui->txtConsole->setStyleSheet("QPlainTextEdit { background-color: #000000; color: #FFFFFF; }");
}

void PartMainConsole::append(QString text)
{
    ui->txtConsole->appendPlainText(text);
}

void PartMainConsole::appendSystem(QString text)
{
    if (!m_configurationConsole->showSystemCommands()) return;
    append(text);
}

void PartMainConsole::append(CommandAttributes commandAttributes)
{
    append(">> " + commandAttributes.commandLine);

    QTextBlock block = lastBlock();
    BlockData *blockData = new CommandBlockData(block.blockNumber(), commandAttributes.commandIndex);
    block.setUserData(blockData);
}

void PartMainConsole::appendFiltered(CommandAttributes commandAttributes)
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

void PartMainConsole::appendResponse(CommandAttributes commandAttributes)
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

int PartMainConsole::appendProgress(QString text)
{
    ProgressBlockData *blockData = new ProgressBlockData(text, m_index++);
    append(blockData->text(0));

    QTextBlock block = lastBlock();
    block.setUserData(blockData);

    return blockData->index();
}

void PartMainConsole::setProgress(int index, int progress)
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

void PartMainConsole::clear()
{
    ui->txtConsole->clear();
}

void PartMainConsole::send()
{
    QString command = ui->cboCommand->currentText().trimmed();

    if (command.isEmpty()) {
        ui->cboCommand->clearEditText();

        return;
    }

    ui->cboCommand->storeText();
    m_configurationConsole->setCommandHistory(ui->cboCommand->items());

    if (command.startsWith(":")) {
        int space = command.indexOf(' ');
        command = command.mid(1).toLower();
        qDebug() << "[FrmMain]" << command << command.mid(0, space - 1);
        if (m_internalCommands.contains(command.mid(0, space - 1))) {
            emit newCommand(command, true);
        }

        return;
    }

    emit newCommand(command, false);
}

bool PartMainConsole::isScrolledToEnd()
{
    return ui->txtConsole->verticalScrollBar()->value()
                        == ui->txtConsole->verticalScrollBar()->maximum();
}

void PartMainConsole::scrollToEnd()
{
    ui->txtConsole->verticalScrollBar()->setValue(
        ui->txtConsole->verticalScrollBar()->maximum());
}

QTextBlock PartMainConsole::lastBlock()
{
    return ui->txtConsole->document()->lastBlock();
}

void PartMainConsole::onClearClicked()
{
    clear();
    emit consoleCleared();
}

void PartMainConsole::onSendClicked()
{
    send();
}

QString PartMainConsole::ProgressBlockData::text(int progress)
{
    return QString("%1 [%2%]").arg(m_text).arg(progress);

    // ##### mode
    // // 100 -> 20, progress = x
    // int scaled = progress / 5;
    // int rest = 20 - scaled;

    // return QString("%1 [%2%3]").arg(m_text).arg(QString("#").repeated(scaled)).arg(QString("_").repeated(rest));
}

void PartMainConsole::setInternalCommands(const QStringList& commands)
{
    m_internalCommands = commands;
}

bool PartMainConsole::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->cboCommand && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QString currentText = ui->cboCommand->currentText();

        // Disable built-in completer for commands starting with ':'
        if (currentText.startsWith(':') && ui->cboCommand->completer()) {
            ui->cboCommand->completer()->setCompletionPrefix("");
        }

        if (keyEvent->key() == Qt::Key_Tab) {
            // Clear any selection from built-in completer
            if (ui->cboCommand->lineEdit() && ui->cboCommand->lineEdit()->hasSelectedText()) {
                QString textBeforeSelection = currentText.left(ui->cboCommand->lineEdit()->selectionStart());
                ui->cboCommand->setEditText(textBeforeSelection);
            }
            handleAutocomplete();

            return true;
        }

        if (keyEvent->key() == Qt::Key_Escape) {
            if (!m_autocompletePrefix.isEmpty()) {
                cancelAutocomplete();

                return true;
            }
        }

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            send();

            return true;
        }

        if (keyEvent->key() == Qt::Key_Up && ui->cboCommand->currentIndex() == 0) {
            ui->cboCommand->setCurrentIndex(-1);
            ui->cboCommand->clearEditText();

            return true;
        }

        cancelAutocomplete();
    }

    return QWidget::eventFilter(watched, event);
}

void PartMainConsole::handleAutocomplete()
{
    QString currentText = ui->cboCommand->currentText();

    if (!currentText.startsWith(':')) {
        return;
    }

    QString prefix = currentText.mid(1);

    // Check if current text is one of our matches - if so, continue cycling
    bool isMatchedCompletion = false;
    if (!m_autocompleteMatches.isEmpty()) {
        for (const QString& match : m_autocompleteMatches) {
            if (prefix == match) {
                isMatchedCompletion = true;
                break;
            }
        }
    }

    // Reset matches if prefix changed and current text is not a match
    if (!isMatchedCompletion && m_autocompletePrefix != prefix) {
        m_autocompletePrefix = prefix;
        m_autocompleteMatches = findMatches(prefix);
        m_autocompleteIndex = -1;
    }

    if (m_autocompleteMatches.isEmpty()) {
        return;
    }

    m_autocompleteIndex = (m_autocompleteIndex + 1) % m_autocompleteMatches.size();
    QString completion = ":" + m_autocompleteMatches[m_autocompleteIndex];
    ui->cboCommand->setEditText(completion);

    if (ui->cboCommand->lineEdit()) {
        ui->cboCommand->lineEdit()->setCursorPosition(completion.length());
    }
}

void PartMainConsole::cancelAutocomplete()
{
    if (!m_autocompletePrefix.isEmpty()) {
        QString restoredText = ":" + m_autocompletePrefix;
        ui->cboCommand->setEditText(restoredText);
        if (ui->cboCommand->lineEdit()) {
            ui->cboCommand->lineEdit()->setCursorPosition(restoredText.length());
        }
    }
    m_autocompletePrefix.clear();
    m_autocompleteIndex = -1;
    m_autocompleteMatches.clear();
}

QStringList PartMainConsole::findMatches(const QString& prefix)
{
    QStringList matches;
    QString lowerPrefix = prefix.toLower();

    for (const QString& cmd : m_internalCommands) {
        if (cmd.startsWith(lowerPrefix)) {
            matches.append(cmd);
        }
    }

    matches.sort();
    return matches;
}
