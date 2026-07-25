#include "comboboxkey.h"

ComboBoxKey::ComboBoxKey(QWidget *parent) : QComboBox(parent)
{
}

// TODO: Rework user items
void ComboBoxKey::setEditable(bool editable)
{
    if (!editable) {
        if (currentText() != itemText(currentIndex())) {
            // Remove user item if exist
            QString value = currentText();
            if (itemData(count() - 1) == 1) {
                removeItem(count() - 1);
                removeItem(count() - 1);
            }

            // Add user item to the end of list
            insertSeparator(count());
            addItem(value, 1);
            setCurrentIndex(count() - 1);
        }
    }

    QComboBox::setEditable(editable);
}

// Separators have empty text and must be skipped. The loops are bounded —
// with a separator as the first / last item the old do-while never ended.
void ComboBoxKey::setCurrentNext()
{
    for (int index = currentIndex() + 1; index < count(); index++) {
        if (!itemText(index).isEmpty()) {
            setCurrentIndex(index);

            return;
        }
    }
}

void ComboBoxKey::setCurrentPrevious()
{
    for (int index = currentIndex() - 1; index >= 0; index--) {
        if (!itemText(index).isEmpty()) {
            setCurrentIndex(index);

            return;
        }
    }
}

void ComboBoxKey::setItems(QStringList items)
{
    if (items.isEmpty()) return;

    clear();

    bool userItem = false;
    foreach (QString item, items) {
        if (item.isEmpty()) {
            insertSeparator(count());
            userItem = true;
        } else {
            insertItem(count(), item, userItem ? 1 : QVariant());
        }
    }
}

QStringList ComboBoxKey::items()
{
    QStringList items;

    for (int i = 0; i < count(); i++) items.append(itemText(i));

    return items;
}

void ComboBoxKey::keyPressEvent(QKeyEvent *e)
{
    if (this->isEditable() || !isBlockedKey(e->key())) QComboBox::keyPressEvent(e);
}

void ComboBoxKey::keyReleaseEvent(QKeyEvent *e)
{
    if (this->isEditable() || !isBlockedKey(e->key())) QComboBox::keyReleaseEvent(e);
}

bool ComboBoxKey::isBlockedKey(int key)
{
    return key != Qt::Key_ScrollLock  && key != Qt::Key_Down && key != Qt::Key_Up;
}
