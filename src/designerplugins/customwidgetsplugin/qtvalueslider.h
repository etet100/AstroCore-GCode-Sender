#ifndef QTVALUESLIDER__HA
#define QTVALUESLIDER__HA

#include <QWidget>
#include "intslider.hpp"
#include "customwidgetsshared.h"

class CUSTOMWIDGETS_DLLSPEC QtValueSlider : public ValueSliders::IntSlider
{
    Q_OBJECT
    Q_PROPERTY(int value READ getVal WRITE setVal)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(QString name WRITE setName READ name)
    Q_PROPERTY(ValueSliders::BoundMode boundMode READ boundMode WRITE setBoundMode)

    public:
        QtValueSlider(QWidget *parent = nullptr);
        void setName(const QString &name) { ValueSliders::IntSlider::setName(name); }
        QString name() const { return name_; }
        void setBoundMode(ValueSliders::BoundMode mode) { boundMode_ = mode; }
        ValueSliders::BoundMode boundMode() const { return boundMode_; }

    private:
        Q_DISABLE_COPY(QtValueSlider)
};

#endif // QTVALUESLIDER__H
