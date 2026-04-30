// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include "partmaincoordinates.h"
#include "ui_partmaincoordinates.h"
#include "core/communicator/machinecoordinatecache.h"
#include <QPushButton>
#include <QHeaderView>
#include "styledtoolbutton.h"

static const QStringList CS_NAMES = {"G54", "G55", "G56", "G57", "G58", "G59"};

PartMainCoordinates::PartMainCoordinates(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainCoordinates)
{
    ui->setupUi(this);
    setupTable();
}

PartMainCoordinates::~PartMainCoordinates()
{
    delete ui;
}

void PartMainCoordinates::setupTable()
{
    ui->tableCoordinates->setRowCount(CS_NAMES.count());
    ui->tableCoordinates->setColumnCount(5);
    ui->tableCoordinates->setHorizontalHeaderLabels({"CS", "X", "Y", "Z", ""});
    ui->tableCoordinates->verticalHeader()->setVisible(false);
    ui->tableCoordinates->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableCoordinates->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for (int i = 0; i < CS_NAMES.count(); i++) {
        auto *itemCS = new QTableWidgetItem(CS_NAMES[i]);
        itemCS->setTextAlignment(Qt::AlignCenter);
        QFont boldFont = itemCS->font();
        boldFont.setBold(true);
        itemCS->setFont(boldFont);
        ui->tableCoordinates->setItem(i, 0, itemCS);

        for (int col = 1; col <= 3; col++) {
            auto *item = new QTableWidgetItem("0.000");
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableCoordinates->setItem(i, col, item);
        }

        auto *btn = new StyledToolButton();
        btn->setIcon(QIcon(":/images/axis_zero.svg"));
        btn->setToolTip(tr("Select %1").arg(CS_NAMES[i]));
        btn->setAutoRaise(true);
        btn->setInvertedDartThemeIconColors(true);
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            emit coordinateSystemSelected(CS_NAMES[i]);
        });
        ui->tableCoordinates->setCellWidget(i, 4, btn);
    }

    auto *header = ui->tableCoordinates->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    header->setDefaultAlignment(Qt::AlignCenter);

    ui->tableCoordinates->setEnabled(false);
}

void PartMainCoordinates::updateFromCache(const MachineCoordinateCache &cache)
{
    if (!ui->tableCoordinates->isEnabled()) {
        ui->tableCoordinates->setEnabled(true);
    }

    for (int i = 0; i < CS_NAMES.count(); i++) {
        QVector3D offset = cache.coords(CS_NAMES[i]);
        ui->tableCoordinates->item(i, 1)->setText(QString::number(offset.x(), 'f', 3));
        ui->tableCoordinates->item(i, 2)->setText(QString::number(offset.y(), 'f', 3));
        ui->tableCoordinates->item(i, 3)->setText(QString::number(offset.z(), 'f', 3));
    }
}

void PartMainCoordinates::setActiveRow(const QString &cs)
{
    if (cs == m_activeCS) {
        return;
    }

    m_activeCS = cs;
    for (int i = 0; i < CS_NAMES.count(); i++) {
        bool active = (CS_NAMES[i] == m_activeCS);
        for (int col = 0; col < 4; col++) {
            ui->tableCoordinates->item(i, col)->setBackground(active ? palette().highlight() : QBrush());
            ui->tableCoordinates->item(i, col)->setForeground(active ? palette().highlightedText() : QBrush());
        }
    }
}
