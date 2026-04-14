#include "macrowidget.h"
#include "utils/utils.h"
#include "ui/utils/thememanager.h"

#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QIcon>
#include <QResizeEvent>


MacroWidget::MacroWidget(QWidget *parent)
    : QFrame(parent)
{
    m_label = new QLabel(this);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_label->setContentsMargins(5, 2, 5, 2);

    m_btnEdit = new QToolButton(this);
    m_btnEdit->setAutoRaise(true);
    m_btnEdit->setToolTip(tr("Edit"));
    m_btnEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    // m_btnEdit->setStyleSheet("background-color: palette(button); border: 0;");

    m_btnRun = new QToolButton(this);
    m_btnRun->setAutoRaise(true);
    m_btnRun->setToolTip(tr("Run"));
    m_btnRun->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    // m_btnRun->setStyleSheet("background-color: palette(button); border: 0;");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addWidget(m_label);
    layout->addWidget(m_btnEdit);
    layout->addWidget(m_btnRun);

    connect(m_btnEdit, &QToolButton::clicked, this, [this]() { emit editClicked(m_id); });
    connect(m_btnRun, &QToolButton::clicked, this, [this]() { emit runClicked(m_id); });

    m_dark = ThemeManager::instance().dark();
    updateIcons();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool dark) {
        if (m_dark != dark) {
            m_dark = dark;
            updateIcons();
        }
    });
}

void MacroWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    int h = m_btnEdit->height();
    if (m_btnEdit->width() != h) {
        m_btnEdit->setFixedWidth(h);
        m_btnRun->setFixedWidth(h);
    }
}

void MacroWidget::setId(int id)
{
    m_id = id;
}

int MacroWidget::id() const
{
    return m_id;
}

void MacroWidget::setName(const QString &name)
{
    m_label->setText(name);
}

QString MacroWidget::name() const
{
    return m_label->text();
}

void MacroWidget::updateIcons()
{
    m_btnEdit->setIcon(QIcon(":/images/settings.svg"));
    m_btnEdit->setIconSize(QSize(16, 16));
    m_btnRun->setIcon(QIcon(":/images/run.svg"));
    m_btnRun->setIconSize(QSize(16, 16));

    if (m_dark) {
        Utils::invertButtonIconColors(m_btnEdit);
        Utils::invertButtonIconColors(m_btnRun);
    }
}
