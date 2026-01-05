#ifndef PARTMAINPROGRAM_H
#define PARTMAINPROGRAM_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QAbstractItemDelegate>
#include <QAbstractItemView>
#include <QModelIndex>
#include "ui/tables/gcodetablemodel.h"
#include "ui/tables/heightmaptablemodel.h"
#include "ui/tables/gcodeitemdelegate.h"

class GCode;
class Heightmap;

namespace Ui {
class PartMainProgram;
}

class PartMainProgram : public QWidget
{
    Q_OBJECT

    public:
        explicit PartMainProgram(QWidget* parent = nullptr);
        ~PartMainProgram();

        void initialize(GCode* program, Heightmap* heightmap);

        void setProgramModel(QAbstractItemModel* model);
        void setProgramItemDelegate(QAbstractItemDelegate* delegate);
        void setHeightMapModel(QAbstractItemModel* model);

        // Table models access
        GCodeTableModel* programModel() { return m_programModel; }
        GCodeTableModel* probeModel() { return m_probeModel; }
        GCodeTableModel* programHeightmapModel() { return m_programHeightmapModel; }
        GCodeTableModel* currentModel() { return m_currentModel; }
        HeightmapTableModel* heightmapModel() { return m_heightmapModel; }

        void setCurrentModel(GCodeTableModel* model);
        void insertRowInCurrentModel(int row);
        void removeRowsFromCurrentModel(int row, int count);
        int currentModelRowCount() const;
        QModelIndex currentModelIndex(int row, int column) const;
        QVariant currentModelData(const QModelIndex& index) const;
        void setCurrentModelData(const QModelIndex& index, const QVariant& value);
        void clearProgramHeightmapModel();
        void clearHeightmapModel();
        void setProgramModelCommentsVisible(bool visible);

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
        void insertLineRequested();
        void deleteLinesRequested();
        void modelDataChanged(QModelIndex i1, QModelIndex i2);

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
        void onTableContextMenuRequested(const QPoint& pos);
        void onInsertLineTriggered();
        void onDeleteLinesTriggered();

    private:
        Ui::PartMainProgram* ui;
        QMenu* m_tableMenu;

        // Table models
        GCodeTableModel* m_programModel;
        GCodeTableModel* m_probeModel;
        GCodeTableModel* m_programHeightmapModel;
        GCodeTableModel* m_currentModel;
        HeightmapTableModel* m_heightmapModel;
        GCodeItemDelegate m_programItemDelegate;

        void setupUi();
        void setupTableContextMenu();
};

#endif // PARTMAINPROGRAM_H
