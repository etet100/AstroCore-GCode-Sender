#include "frmlog.h"
#include "ui_frmlog.h"

#include <QAbstractItemModel>
#include <QPainter>
#include <QTime>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QScrollBar>
#include <QHeaderView>
#include <QMouseEvent>

struct CategoryItem
{
    QString name;
    QString pathKey;  // "A" or "A|B" — used as flat index key
    Qt::CheckState checkState = Qt::Checked;
    CategoryItem* parent = nullptr;
    QList<CategoryItem*> children;

    ~CategoryItem() { qDeleteAll(children); }
};

class TagTree
{
    public:
        TagTree()
        {
            m_root = new CategoryItem();
            m_root->name = "__root__";
        }

        ~TagTree() { delete m_root; }

        CategoryItem* root() const { return m_root; }

        // Unknown paths are treated as visible.
        // Empty path maps to the "-- no tag --" entry.
        // Checks the full ancestor chain — any unchecked level hides the item.
        bool isVisible(const QStringList& path) const
        {
            if (path.isEmpty()) {
                CategoryItem* item = m_index.value("-- no tag --", nullptr);
                return !item || item->checkState != Qt::Unchecked;
            }
            QString key;
            for (const QString& tag : path) {
                if (!key.isEmpty()) {
                    key += '|';
                }
                key += tag;
                CategoryItem* item = m_index.value(key, nullptr);
                if (item && item->checkState == Qt::Unchecked) {
                    return false;
                }
            }

            return true;
        }

        CategoryItem* findChild(CategoryItem* parent, const QString& name) const
        {
            for (auto* c : parent->children) {
                if (c->name == name) {
                    return c;
                }
            }

            return nullptr;
        }

        // Caller must wrap in beginInsertRows / endInsertRows.
        CategoryItem* createChild(CategoryItem* parent, const QString& name, const QString& pathKey, int row,
                                  Qt::CheckState initialState = Qt::Checked)
        {
            auto* item = new CategoryItem();
            item->name = name;
            item->pathKey = pathKey;
            item->checkState = initialState;
            item->parent = parent;
            parent->children.insert(row, item);
            m_index[pathKey] = item;

            return item;
        }

    private:
        CategoryItem* m_root;
        QHash<QString, CategoryItem*> m_index;
};

class CategoriesModel : public QAbstractItemModel
{
    public:
        explicit CategoriesModel(TagTree* tree, QObject* parent = nullptr)
            : QAbstractItemModel(parent)
            , m_tree(tree)
        {}

        bool ensurePath(const QStringList& path)
        {
            CategoryItem* current = m_tree->root();
            QString key;
            bool added = false;

            for (const QString& tag : path) {
                if (!key.isEmpty()) {
                    key += '|';
                }
                key += tag;

                CategoryItem* child = m_tree->findChild(current, tag);
                if (!child) {
                    QString cached = Cache::instance().get(key);
                    Qt::CheckState initialState;
                    if (cached.isEmpty()) {
                        initialState = Qt::Checked;
                        Cache::instance().set(key, true);
                        Cache::instance().flush();
                    } else {
                        initialState = (cached == "1") ? Qt::Checked : Qt::Unchecked;
                    }
                    int row = 0;
                    while (row < current->children.size() && current->children[row]->name < tag) {
                        row++;
                    }
                    beginInsertRows(indexForItem(current), row, row);
                    child = m_tree->createChild(current, tag, key, row, initialState);
                    added = true;
                    endInsertRows();
                }
                current = child;
            }

            return added;
        }

        void setAll(Qt::CheckState state)
        {
            setAllRecursive(m_tree->root(), state);
            beginResetModel();
            endResetModel();
        }

        void toggleAll()
        {
            toggleAllRecursive(m_tree->root());
            beginResetModel();
            endResetModel();
        }

        void saveAllToCache() const
        {
            saveAllRecursive(m_tree->root());
            Cache::instance().flush();
        }

        QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override
        {
            if (!hasIndex(row, column, parent)) {
                return {};
            }
            CategoryItem* p = parent.isValid()
                ? static_cast<CategoryItem*>(parent.internalPointer())
                : m_tree->root();
            if (row < p->children.size()) {
                return createIndex(row, column, p->children[row]);
            }

            return {};
        }

        QModelIndex parent(const QModelIndex& child) const override
        {
            if (!child.isValid()) {
                return {};
            }
            auto* item = static_cast<CategoryItem*>(child.internalPointer());
            CategoryItem* p = item->parent;
            if (!p || p == m_tree->root()) {
                return {};
            }
            CategoryItem* gp = p->parent;
            if (!gp) {
                return {};
            }

            return createIndex(gp->children.indexOf(p), 0, p);
        }

        int rowCount(const QModelIndex& parent = {}) const override
        {
            CategoryItem* p = parent.isValid()
                ? static_cast<CategoryItem*>(parent.internalPointer())
                : m_tree->root();

            return p->children.size();
        }

        int columnCount(const QModelIndex& = {}) const override { return 2; }

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
        {
            if (!index.isValid()) {
                return {};
            }
            auto* item = static_cast<CategoryItem*>(index.internalPointer());
            if (index.column() == 0 && role == Qt::DisplayRole) {
                return item->name;
            }
            if (index.column() == 1 && role == Qt::CheckStateRole) {
                return item->checkState;
            }

            return {};
        }

        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
        {
            if (!index.isValid() || index.column() != 1 || role != Qt::CheckStateRole) {
                return false;
            }
            auto* item = static_cast<CategoryItem*>(index.internalPointer());
            auto state = static_cast<Qt::CheckState>(value.toInt());
            propagateDown(item, state);
            if (state == Qt::Checked) {
                checkAncestors(item);
            }

            return true;
        }

        Qt::ItemFlags flags(const QModelIndex& index) const override
        {
            if (!index.isValid()) {
                return Qt::NoItemFlags;
            }
            if (index.column() == 1) {
                return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
            }

            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }

    private:
        TagTree* m_tree;

        void setAllRecursive(CategoryItem* item, Qt::CheckState state)
        {
            for (auto* child : item->children) {
                child->checkState = state;
                setAllRecursive(child, state);
            }
        }

        void toggleAllRecursive(CategoryItem* item)
        {
            for (auto* child : item->children) {
                child->checkState = (child->checkState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
                toggleAllRecursive(child);
            }
        }

        void saveAllRecursive(CategoryItem* item) const
        {
            for (auto* child : item->children) {
                Cache::instance().set(child->pathKey, child->checkState != Qt::Unchecked);
                saveAllRecursive(child);
            }
        }

        QModelIndex indexForItem(CategoryItem* item, int column = 0) const
        {
            if (!item || item == m_tree->root()) {
                return {};
            }
            CategoryItem* p = item->parent;
            if (!p) {
                return {};
            }

            return createIndex(p->children.indexOf(item), column, item);
        }

        void propagateDown(CategoryItem* item, Qt::CheckState state)
        {
            item->checkState = state;
            QModelIndex idx = indexForItem(item, 1);
            emit dataChanged(idx, idx, {Qt::CheckStateRole});
            if (state == Qt::Unchecked) {
                for (auto* child : item->children) {
                    propagateDown(child, state);
                }
            }
        }

        void checkAncestors(CategoryItem* item)
        {
            for (CategoryItem* p = item->parent; p && p != m_tree->root(); p = p->parent) {
                if (p->checkState == Qt::Checked) {
                    break;
                }
                p->checkState = Qt::Checked;
                QModelIndex idx = indexForItem(p, 1);
                emit dataChanged(idx, idx, {Qt::CheckStateRole});
            }
        }
};

class CategoryDelegate : public QStyledItemDelegate
{
    public:
        explicit CategoryDelegate(QObject* parent = nullptr)
            : QStyledItemDelegate(parent)
        {}

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            QStyleOptionViewItem opt = option;
            initStyleOption(&opt, index);

            auto checkState = static_cast<Qt::CheckState>(
                index.model()->data(index.siblingAtColumn(1), Qt::CheckStateRole).toInt());

            painter->save();

            if (opt.state & QStyle::State_Selected) {
                painter->fillRect(opt.rect, opt.palette.color(QPalette::Highlight));
                opt.state &= ~QStyle::State_Selected;
                opt.palette.setColor(QPalette::Text, opt.palette.color(QPalette::HighlightedText));
            } else if (opt.state & QStyle::State_MouseOver) {
                QColor hover = opt.palette.color(QPalette::Highlight);
                hover.setAlpha(60);
                painter->fillRect(opt.rect, hover);
            } else if (checkState == Qt::Checked) {
                painter->fillRect(opt.rect, opt.palette.color(QPalette::AlternateBase));
            }

            painter->restore();
            QStyledItemDelegate::paint(painter, opt, index);
        }
};

FrmLog::FrmLog() : QDialog()
    , ui(new Ui::FrmLog)
    , m_tagTree(new TagTree())
    , m_categoriesModel(new CategoriesModel(m_tagTree, this))
    , m_regenerateTimer(new QTimer(this))
{
    // Ensure the dialog has minimize/maximize/close buttons and can be alt-tabbed,
    // but is still a tool window without a taskbar entry.
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint |
               Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

    ui->setupUi(this);
    ui->treeCategories->setHeaderHidden(true);
    ui->treeCategories->setModel(m_categoriesModel);
    ui->treeCategories->setItemDelegate(new CategoryDelegate(this));
    ui->treeCategories->setMouseTracking(true);
    ui->treeCategories->header()->setStretchLastSection(false);
    ui->treeCategories->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeCategories->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->treeCategories->header()->resizeSection(1, 24);
    ui->treeCategories->viewport()->installEventFilter(this);

    connect(m_categoriesModel, &QAbstractItemModel::rowsInserted,
            this, [this](const QModelIndex& parent, int, int) {
        ui->treeCategories->expand(parent);
    });

    connect(m_categoriesModel, &QAbstractItemModel::dataChanged,
            this, [this](const QModelIndex& topLeft, const QModelIndex&, const QList<int>& roles) {
        if (!roles.contains(Qt::CheckStateRole)) {
            return;
        }
        auto* item = static_cast<CategoryItem*>(topLeft.internalPointer());
        Cache::instance().set(item->pathKey, item->checkState != Qt::Unchecked);
        Cache::instance().flush();
        regenerateWithDelay();
    });

    QPushButton* resetBtn = ui->buttonBox->button(QDialogButtonBox::Reset);
    resetBtn->setText("&Clear log");
    connect(resetBtn, &QPushButton::clicked, this, &FrmLog::clear);

    connect(ui->txtIncludeText, &QLineEdit::textChanged, this, [this]() {
        Cache::instance().set("include-text", ui->txtIncludeText->text());
        Cache::instance().flush();
        regenerateWithDelay();
    });
    connect(ui->txtExcludeText, &QLineEdit::textChanged, this, [this]() {
        Cache::instance().set("exclude-text", ui->txtExcludeText->text());
        Cache::instance().flush();
        regenerateWithDelay();
    });

    connect(ui->btnClearIncludeText, &QToolButton::clicked, this, [this]() {
        ui->txtIncludeText->clear();
    });
    connect(ui->btnClearExcludeText, &QToolButton::clicked, this, [this]() {
        ui->txtExcludeText->clear();
    });

    m_regenerateTimer->setSingleShot(true);
    m_regenerateTimer->setInterval(300);
    connect(m_regenerateTimer, &QTimer::timeout, this, &FrmLog::regenerateLog);

    m_categoriesModel->ensurePath({"-- no tag --"});

    const QString savedIncludeText = Cache::instance().get("include-text");
    if (!savedIncludeText.isEmpty()) {
        ui->txtIncludeText->setText(savedIncludeText);
    }
    const QString savedExcludeText = Cache::instance().get("exclude-text");
    if (!savedExcludeText.isEmpty()) {
        ui->txtExcludeText->setText(savedExcludeText);
    }

    connect(ui->btnTreeAll, &QPushButton::clicked, this, &FrmLog::treeSelectAll);
    connect(ui->btnTreeNone, &QPushButton::clicked, this, &FrmLog::treeSelectNone);
    connect(ui->btnTreeToggle, &QPushButton::clicked, this, &FrmLog::treeToggleSelection);

    connect(ui->cmbMinLevel, &QComboBox::currentIndexChanged, this, [this](int index) {
        Cache::instance().set("min-level", index);
        Cache::instance().flush();
        regenerateWithDelay();
    });

    ui->cmbMinLevel->setCurrentIndex(Cache::instance().getInt("min-level", 0));
}

void FrmLog::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

FrmLog::~FrmLog()
{
    delete ui;
    delete m_tagTree;
}

bool FrmLog::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->treeCategories->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        auto* me = static_cast<QMouseEvent*>(event);
        QModelIndex idx = ui->treeCategories->indexAt(me->pos());
        if (idx.isValid() && idx.column() == 0) {
            QModelIndex checkIdx = m_categoriesModel->index(idx.row(), 1, idx.parent());
            auto current = static_cast<Qt::CheckState>(m_categoriesModel->data(checkIdx, Qt::CheckStateRole).toInt());
            m_categoriesModel->setData(checkIdx, current == Qt::Checked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);

            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void FrmLog::log(QtMsgType type, const QString& msg)
{
    const QString timestamped = QTime::currentTime().toString("hh:mm:ss ") + msg;

    QStringList tags = findTags(msg);
    m_entries.append({tags, timestamped, type});

    if (!m_tagTree->isVisible(tags)) {
        return;
    }
    if (!passesFilter(msg, type)) {
        return;
    }

    appendEntry(m_entries.last());
}

void FrmLog::regenerateWithDelay()
{
    m_regenerateTimer->start();
}

void FrmLog::removeOld(int linesCount)
{
    int toRemove = m_entries.size() - linesCount;
    if (toRemove <= 0) {
        return;
    }
}

void FrmLog::treeSelectAll()
{
    m_categoriesModel->setAll(Qt::Checked);
    ui->treeCategories->expandAll();
    m_categoriesModel->saveAllToCache();
    regenerateWithDelay();
}

void FrmLog::treeSelectNone()
{
    m_categoriesModel->setAll(Qt::Unchecked);
    ui->treeCategories->expandAll();
    m_categoriesModel->saveAllToCache();
    regenerateWithDelay();
}

void FrmLog::treeToggleSelection()
{
    m_categoriesModel->toggleAll();
    ui->treeCategories->expandAll();
    m_categoriesModel->saveAllToCache();
    regenerateWithDelay();
}

void FrmLog::clear()
{
    m_entries.clear();
    ui->txtLog->clear();
}

void FrmLog::regenerateLog()
{
    bool isAtEnd = isScrolledToEnd();
    ui->txtLog->clear();

    for (const LogEntry& entry : m_entries) {
        if (!m_tagTree->isVisible(entry.tags)) {
            continue;
        }
        if (!passesFilter(entry.text, entry.type)) {
            continue;
        }
        appendEntry(entry);
    }

    if (isAtEnd) {
        scrollToEnd();
    }
}

bool FrmLog::isScrolledToEnd()
{
    return ui->txtLog->verticalScrollBar()->value() == ui->txtLog->verticalScrollBar()->maximum();
}

void FrmLog::scrollToEnd()
{
    ui->txtLog->verticalScrollBar()->setValue(ui->txtLog->verticalScrollBar()->maximum());
}

void FrmLog::appendEntry(const LogEntry& entry)
{
    QString color;
    switch (entry.type) {
        case QtDebugMsg:
            ui->txtLog->appendPlainText(entry.text);
            return;
        case QtWarningMsg:
            color = "orange";
            break;
        case QtCriticalMsg:
            color = "red";
            break;
        case QtInfoMsg:
            color = "lightblue";
            break;
        case QtFatalMsg:
            color = "darkred";
            break;
    }

    bool isAtEnd = isScrolledToEnd();

    ui->txtLog->appendHtml(QString("<span style=\"color:%1;\">%2</span>").arg(color, entry.text.toHtmlEscaped()));

    if (isAtEnd) {
        scrollToEnd();
    }
}

QStringList FrmLog::findTags(const QString& msg)
{
    QStringList tags;
    int pos = 0;

    while (pos < msg.length() && msg[pos] == '[') {
        int close = msg.indexOf(']', pos + 1);
        if (close == -1) {
            break;
        }
        tags.append(msg.mid(pos + 1, close - pos - 1));
        pos = close + 1;
    }

    if (!tags.isEmpty()) {
        m_categoriesModel->ensurePath(tags);
    }

    return tags;
}

static int msgSeverity(QtMsgType type)
{
    switch (type) {
        case QtDebugMsg:    return 0;
        case QtInfoMsg:     return 1;
        case QtWarningMsg:  return 2;
        case QtCriticalMsg: return 3;
        case QtFatalMsg:    return 4;
    }
    return 0;
}

bool FrmLog::passesFilter(const QString& text, QtMsgType type) const
{
    if (msgSeverity(type) < ui->cmbMinLevel->currentIndex()) {
        return false;
    }

    const QString include = ui->txtIncludeText->text().trimmed();
    if (!include.isEmpty() && !text.contains(include, Qt::CaseInsensitive)) {
        return false;
    }

    const QString excludeRaw = ui->txtExcludeText->text();
    if (!excludeRaw.isEmpty()) {
        for (const QString& part : excludeRaw.split('|')) {
            const QString exclude = part.trimmed();
            if (!exclude.isEmpty() && text.contains(exclude, Qt::CaseInsensitive)) {
                return false;
            }
        }
    }

    return true;
}
