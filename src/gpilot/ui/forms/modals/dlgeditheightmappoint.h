#ifndef DLGEDITHEIGHTMAPPOINT_H
#define DLGEDITHEIGHTMAPPOINT_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class DlgEditHeightmapPoint;
}

class DlgEditHeightmapPoint : public QDialog
{
    Q_OBJECT

    public:
        explicit DlgEditHeightmapPoint(QPoint point, double height, QWidget* parent = nullptr);
        ~DlgEditHeightmapPoint();

        QPoint point() const { return m_point; }
        double height() const;

    protected:
        void showEvent(QShowEvent* event) override;

    private slots:
        void dialogButtonClick(QAbstractButton*);

    private:
        Ui::DlgEditHeightmapPoint* ui;
        QPoint m_point;
        double m_orgHeight;
};

#endif // DLGEDITHEIGHTMAPPOINT_H
