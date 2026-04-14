#ifndef FRMLOG_H
#define FRMLOG_H

#include <QCloseEvent>
#include <QDialog>
#include <QStringList>
#include <QTimer>
#include "cache.h"

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
    explicit FrmLog(bool doNotClose);
    ~FrmLog();
    void log(QtMsgType type, const QString& msg);

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
    bool m_doNotClose;

    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void regenerateLog();
    QStringList findTags(const QString& msg);
    void appendEntry(const LogEntry& entry);
    bool passesFilter(const QString& text, QtMsgType type) const;
    void removeOld(int maxEntries);
    void saveWindowState() const;
    void restoreWindowState();
    int maxEntries() const;
};

#endif // FRMLOG_H
