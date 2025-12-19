#ifndef QTVALUESLIDERPLUGIN_H
#define QTVALUESLIDERPLUGIN_H

#include <QObject>
#include <QDesignerCustomWidgetInterface>

class QtValueSliderPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

    public:
        explicit QtValueSliderPlugin(QObject *parent);

        bool isContainer() const override;
        bool isInitialized() const override;
        QIcon icon() const override;
        QString domXml() const override;
        QString group() const override;
        QString includeFile() const override;
        QString name() const override;
        QString toolTip() const override;
        QString whatsThis() const override;
        QWidget *createWidget(QWidget *parent) override;
        void initialize(QDesignerFormEditorInterface *core) override;

    private:
        bool m_initialized;
};

#endif // QTVALUESLIDERPLUGIN_H
