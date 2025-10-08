// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINCONSOLE_H
#define PARTMAINCONSOLE_H

#include <QWidget>
#include <QTextBlock>
#include <QAbstractListModel>
#include "../../config/module/configurationconsole.h"
#include "globals.h"

namespace Ui {
class partMainConsole;
}

class partMainConsole : public QWidget
{
    Q_OBJECT

    public:
        explicit partMainConsole(QWidget *parent);
        void initialize(ConfigurationConsole &configurationConsole);
        ~partMainConsole();
        void append(QString text);
        void appendSystem(QString text);
        void append(CommandAttributes commandAttributes);
        void appendFiltered(CommandAttributes commandAttributes);
        void appendResponse(CommandAttributes commandAttributes);
        int appendProgress(QString text);
        void setProgress(int index, int progress);
        void clear();

    signals:
        void newCommand(QString command);
        void consoleCleared();

    private:
        static const int MAX_BLOCKS_TO_CHECK = 100;

        Ui::partMainConsole *ui;
        ConfigurationConsole *m_configurationConsole;
        void send();
        bool isScrolledToEnd();
        void scrollToEnd();
        QTextBlock lastBlock();

        enum ItemType {
            Command,
            Progress,
        };

        class BlockData : public QTextBlockUserData
        {
            public:
                BlockData(ItemType it) : m_it(it) {}
                ItemType type() const { return m_it; }

            private:
                ItemType m_it;
        };

        class CommandBlockData : public BlockData
        {
            public:
                CommandBlockData(int consoleIndex, int commandIndex) : BlockData(ItemType::Command), m_consoleIndex(consoleIndex), m_commandIndex(commandIndex) {}
                int consoleIndex() const { return m_consoleIndex; }
                int commandIndex() const { return m_commandIndex; }

            private:
                int m_consoleIndex;
                int m_commandIndex;
        };

        class ProgressBlockData : public BlockData
        {
            public:
                ProgressBlockData(QString text, int index) : BlockData(ItemType::Progress),  m_index(index), m_text(text) {}
                int index() const { return m_index; }
                QString text(int progress);

            private:
                int m_index;
                QString m_text;
        };

        void applyDarkBackgroundMode();

        int m_index = 0;

    private slots:
        void onClearClicked();
        void onSendClicked();        
};

#endif // PARTMAINCONSOLE_H
