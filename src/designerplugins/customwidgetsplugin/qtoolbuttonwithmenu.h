#ifndef QTOOLBUTTONWITHMENU_H
#define QTOOLBUTTONWITHMENU_H

#include <QToolButton>
#include <QIcon>
#include "customwidgetsshared.h"

class QMenu;

class CUSTOMWIDGETS_DLLSPEC QToolButtonWithMenu : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QIcon menuIndicatorIcon READ menuIndicatorIcon WRITE setMenuIndicatorIcon)
    Q_PROPERTY(double indicatorScaleFactor READ indicatorScaleFactor WRITE setIndicatorScaleFactor)
    Q_PROPERTY(int menuIndicatorMargin READ menuIndicatorMargin WRITE setMenuIndicatorMargin)

public:
    explicit QToolButtonWithMenu(QWidget *parent = nullptr);

    QIcon menuIndicatorIcon() const;
    void setMenuIndicatorIcon(const QIcon &icon);

    double indicatorScaleFactor() const;
    void setIndicatorScaleFactor(double factor);

    int menuIndicatorMargin() const;
    void setMenuIndicatorMargin(int margin);

    QMenu *buttonMenu() const;
    void setButtonMenu(QMenu *menu);

signals:
    void menuRequested();

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    bool hasButtonMenu() const;
    Q_DISABLE_COPY(QToolButtonWithMenu)

    int indicatorSize() const;
    QRect menuIndicatorRect() const;
    bool isMenuIndicatorClick(const QPoint &pos) const;
    void showButtonMenu();

    QIcon m_menuIndicatorIcon;
    double m_indicatorScaleFactor = 1.0;
    int m_menuIndicatorMargin = 1;
    QMenu *m_menu = nullptr;
    bool m_menuIndicatorPressed = false;
    bool m_menuIndicatorHovered = false;
};

#endif // QTOOLBUTTONWITHMENU_H
