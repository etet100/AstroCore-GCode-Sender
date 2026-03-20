#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <QColor>
#include <QIcon>
#include <QWidget>
#include "customwidgetsshared.h"

class CUSTOMWIDGETS_DLLSPEC IconLabel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QString svgIcon READ svgIcon WRITE setSvgIcon)
    Q_PROPERTY(QColor iconColor READ iconColor WRITE setIconColor)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(bool invertIconColors READ invertIconColors WRITE setInvertIconColors)

public:
    explicit IconLabel(QWidget *parent = nullptr);

    QString text() const;
    void setText(const QString &text);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString svgIcon() const;
    void setSvgIcon(const QString &filePath);

    QColor iconColor() const;
    void setIconColor(const QColor &color);  // equivalent to setStyleSheet("color: ...")

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    int spacing() const;
    void setSpacing(int spacing);

    bool invertIconColors() const;
    void setInvertIconColors(bool invert);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void changeEvent(QEvent *e) override;

private:
    QImage renderSvg() const;

    QString m_text;
    QIcon m_icon;
    QString m_svgPath;
    QColor m_iconColor;           // invalid = use palette (follows stylesheet color:)
    QSize m_iconSize = {16, 16};
    int m_spacing = 4;
    bool m_invertIconColors = true;
};

#endif // ICONLABEL_H
