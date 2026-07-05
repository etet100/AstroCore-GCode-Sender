// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include <QDebug>
#include <QKeyEvent>
#include "combobox.h"

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent)
{
}

ComboBox::~ComboBox()
{
}

void ComboBox::storeText()
{
    if (this->count() == this->maxCount()) {
        this->removeItem(this->maxCount() - 1);
    }
    this->insertItem(0, this->currentText());
    this->setCurrentIndex(-1);
}

void ComboBox::addItems(const QStringList &texts)
{
    QComboBox::addItems(texts);
}

void ComboBox::setItems(const QStringList &texts)
{
    QComboBox::clear(); addItems(texts);
}

QStringList ComboBox::items()
{
    QStringList l;
    for (int i = 0; i < count(); i++) {
        l << itemText(i);
    }

    return l;
}

void ComboBox::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return) {
        if (this->count() == this->maxCount()) this->removeItem(this->maxCount() - 1);  // TODO: Make removing selected item
    }
    QComboBox::keyPressEvent(e);

    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        emit returnPressed();
        this->setCurrentIndex(-1);
    }
}
