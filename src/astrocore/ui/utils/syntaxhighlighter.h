#ifndef GUI_SYNTAXHIGHLIGHTER_H
#define GUI_SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class TextEditor;

class SyntaxHighlighter: public QSyntaxHighlighter
{
    public:
        SyntaxHighlighter(QTextDocument *parent);
        ~SyntaxHighlighter() override;

    protected:
        void highlightBlock(const QString &text) override;

        enum ElementType
        {
            GCommand = 0,
            MCommand = 1,
            Comment = 2,
            FlowControl = 3,
            Comparison = 4,
            Location = 5,
            Function = 6,
            Variable = 7,
            PositionAlpha = 8,
            PositionNumeric = 9,
            ArcPosition = 10,
            FeedSpeed = 11,
            OnNumber = 12,
            Bracket = 13
        };

        void setColor(ElementType type, const QColor& color);
        QColor colorByType(ElementType type);

    private:
        struct HighlightingRule
        {
            QRegularExpression pattern;
            ElementType type;
            QTextCharFormat format;
        };

        QList<HighlightingRule> m_rules;
        void colorChanged(ElementType type, const QColor& col);
};

#endif  // GUI_SYNTAXHIGHLIGHTER_H
