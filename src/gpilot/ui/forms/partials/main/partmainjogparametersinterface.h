#ifndef JOGPARAMETERSCONTROLINTERFACE_H
#define JOGPARAMETERSCONTROLINTERFACE_H

#include <QFrame>

class PartMainJogParametersInterface : public QFrame
{
    Q_OBJECT

public:
    explicit PartMainJogParametersInterface(QWidget *parent = nullptr) : QFrame(parent) {}
    virtual ~PartMainJogParametersInterface() = default;

    virtual void setStepSizeOptions(const QStringList& options) = 0;
    virtual void setFeedRateXYOptions(const QStringList& options) = 0;
    virtual void setFeedRateZOptions(const QStringList& options) = 0;

    virtual void setStepSize(float value) = 0;
    virtual void setFeedRateXY(float value) = 0;
    virtual void setFeedRateZ(float value) = 0;

    virtual void setSeparateZFeedrate(bool enabled) = 0;

    virtual float stepSize() const = 0;
    virtual float feedRateXY() const = 0;
    virtual float feedRateZ() const = 0;

signals:
    void stepSizeChanged(float value);
    void feedRateXYChanged(float value);
    void feedRateZChanged(float value);
};

#endif // JOGPARAMETERSCONTROLINTERFACE_H
