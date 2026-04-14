#ifndef MACROWIDGET_H
#define MACROWIDGET_H

#include <QFrame>

class QLabel;
class QToolButton;

class MacroWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MacroWidget(QWidget *parent = nullptr);

    void setId(int id);
    int id() const;

    void setName(const QString &name);
    QString name() const;

signals:
    void editClicked(int id);
    void runClicked(int id);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateIcons();

    QLabel *m_label;
    QToolButton *m_btnEdit;
    QToolButton *m_btnRun;
    bool m_dark = false;
    int m_id = -1;
};

#endif // MACROWIDGET_H
