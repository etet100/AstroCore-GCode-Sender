#ifndef XSWITCHBUTTONPLUGIN_H
#define XSWITCHBUTTONPLUGIN_H

#include <QDesignerCustomWidgetInterface>
#include <QObject>

class XSwitchButtonPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

    public:
        explicit XSwitchButtonPlugin(QObject *parent);

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
        bool m_initialized = false;
};

#endif // XSWITCHBUTTONPLUGIN_H
