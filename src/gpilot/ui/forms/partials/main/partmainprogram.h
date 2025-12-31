#ifndef PARTMAINPROGRAM_H
#define PARTMAINPROGRAM_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QAbstractItemDelegate>
#include <QAbstractItemView>
#include <QModelIndex>

namespace Ui {
class PartMainProgram;
}

class PartMainProgram : public QWidget
{
    Q_OBJECT

    public:
        explicit PartMainProgram(QWidget* parent = nullptr);
        ~PartMainProgram();

        void setProgramModel(QAbstractItemModel* model);
        void setProgramItemDelegate(QAbstractItemDelegate* delegate);
        void setHeightMapModel(QAbstractItemModel* model);

        void setAutoScroll(bool enabled);
        bool isAutoScroll() const;

        void setHideComments(bool enabled);
        bool isHideComments() const;

        void setHeightMapVisible(bool visible);
        void setProgramVisible(bool visible);
        void resizeHeightMapSections();
        QByteArray saveHeaderState() const;
        void restoreHeaderState(const QByteArray& state);

        void setFileButtonsEnabled(bool open, bool reset, bool send, bool pause, bool abort);
        void setOpenButtonEnabled(bool enabled);
        void setResetButtonEnabled(bool enabled);
        void setSendButtonEnabled(bool enabled);
        void setAbortButtonEnabled(bool enabled);
        void setPauseButtonEnabled(bool enabled);
        void setPauseButtonText(const QString& text);
        void setPauseButtonChecked(bool checked);
        void setPauseButtonFocus();
        void setSendButtonText(const QString& text);
        void updateButtonStyles();
        void setSendMenuFirstActionEnabled(bool enabled);

        void selectFirstRow();
        void resetToFirstRow();
        void setTableUpdatesEnabled(bool enable);
        void scrollToCurrentIndex(const QModelIndex& index);
        void setProgramTableModel(QAbstractItemModel* model);
        void setProgramTableEditTriggers(QAbstractItemView::EditTriggers triggers);
        QByteArray saveProgramHeaderState() const;
        void setCurrentIndex(const QModelIndex& index);

        // void setupFileOpenMenu(QObject* receiver, const char* openGCodeSlot, const char* openHeightmapSlot);
        void setupFileSendMenu(QObject* receiver, const char* sendFromLineSlot);
        void showTableContextMenu(const QPoint& pos, QMenu* menu, bool hasSelection, int selectedRow, int totalRows);

        QModelIndexList getSelectedRows() const;
        int getFirstSelectedRow() const;
        void selectRow(int row);
        void setRecentFiles(QStringList files);

    signals:
        void abort();
        void start();
        void openFile(QString filePath = "");
        void reset();
        void pause(bool checked = false);
        void clearRecentFiles();
        void currentChanged(const QModelIndex& current, const QModelIndex& previous);
        void hideCommentsChanged(bool checked);
        void manualScrollRequested();
        void customContextMenuRequested(const QPoint& pos);

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    private slots:
        void onScrollBarAction(int action);
        void abortClicked();
        void startClicked();
        void openClicked();
        void resetClicked();
        void pauseClicked(bool checked = false);
        void openRecentFile();

    private:
        Ui::PartMainProgram* ui;
        void setupUi();
};

#endif // PARTMAINPROGRAM_H
