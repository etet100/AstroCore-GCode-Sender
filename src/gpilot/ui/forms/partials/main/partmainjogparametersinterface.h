#ifndef JOGPARAMETERSCONTROLINTERFACE_H
#define JOGPARAMETERSCONTROLINTERFACE_H

#include <QFrame>

class partMainJogParametersInterface : public QFrame
{
    Q_OBJECT

public:
    explicit partMainJogParametersInterface(QWidget *parent = nullptr) : QFrame(parent) {}
    virtual ~partMainJogParametersInterface() = default;

    // Configuration of available options
    virtual void setStepSizeOptions(const QList<float>& options) = 0;
    virtual void setFeedRateXYOptions(const QList<float>& options) = 0;
    virtual void setFeedRateZOptions(const QList<float>& options) = 0;

    // Setting current values
    virtual void setStepSize(float value) = 0;
    virtual void setFeedRateXY(float value) = 0;
    virtual void setFeedRateZ(float value) = 0;

    // Configuration of visibility/behavior
    virtual void setSeparateZFeedrate(bool enabled) = 0;
    virtual bool isSeparateZFeedrate() const = 0;

    // Getting current values
    virtual float getStepSize() const = 0;
    virtual float getFeedRateXY() const = 0;
    virtual float getFeedRateZ() const = 0;

signals:
    void stepSizeChanged(float value);
    void feedRateXYChanged(float value);
    void feedRateZChanged(float value);
};

#endif // JOGPARAMETERSCONTROLINTERFACE_H
