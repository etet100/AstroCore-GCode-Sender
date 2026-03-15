#ifndef FRMLOG_H
#define FRMLOG_H

#include <QCloseEvent>
#include <QDialog>
#include <QStringList>
#include <QTimer>
#include "utils/cache.h"

namespace Ui {
class FrmLog;
}

class CategoriesModel;
class TagTree;

struct LogEntry
{
    QStringList tags;
    QString text;
    QtMsgType type;
};

class FrmLog : public QDialog
{
    Q_OBJECT

public:
    explicit FrmLog();
    ~FrmLog();
    void log(QtMsgType type, const QString& msg);
    void removeOld(int linesCount);

private slots:
    void clear();
    void regenerateWithDelay();
    bool isScrolledToEnd();
    void scrollToEnd();
    void treeSelectAll();
    void treeSelectNone();
    void treeToggleSelection();

private:
    Ui::FrmLog* ui;
    TagTree* m_tagTree;
    CategoriesModel* m_categoriesModel;
    QList<LogEntry> m_entries;
    QTimer* m_regenerateTimer;

    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void regenerateLog();
    QStringList findTags(const QString& msg);
    void appendEntry(const LogEntry& entry);
    bool passesFilter(const QString& text, QtMsgType type) const;
};

#endif // FRMLOG_H
