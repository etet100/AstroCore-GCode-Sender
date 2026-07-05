#include <QApplication>
#include <QPalette>

#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    // comments (inline)
    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::darkGreen);
    commentFormat.setFontItalic(true);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(\(.*\))"),
        Comment,
        commentFormat
    });

    // comments (semicolon)
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(;.*$)"),
        Comment,
        commentFormat
    });

    // flow control operators
    QTextCharFormat flowControlFormat;
    flowControlFormat.setForeground(Qt::darkBlue);
    flowControlFormat.setFontWeight(QFont::Bold);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"((IF|THEN|AND|OR|WHILE|GOTO))"),
        FlowControl,
        flowControlFormat
    });

    // comparison operators
    QTextCharFormat comparisonFormat;
    comparisonFormat.setForeground(Qt::darkRed);
    comparisonFormat.setFontWeight(QFont::Bold);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"((GT|LT|GE|LE|EQ))"),
        Comparison,
        comparisonFormat
    });

    // flow control operators - locations
    QTextCharFormat locationFormat;
    locationFormat.setFontItalic(true);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"((?<=GOTO|DO)\d+)"),
        Location,
        locationFormat
    });

    // functions
    QTextCharFormat functionFormat;
    functionFormat.setForeground(Qt::darkCyan);
    functionFormat.setFontWeight(QFont::Bold);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"((ABS|ACOS|ASIN|ATAN|COS|LN|EXP|FIX|FUP|MOD|ROUND|SQRT|SIN|TAN))"),
        Function,
        functionFormat
    });

    // g commands
    QTextCharFormat gFormat;
    gFormat.setForeground(Qt::blue);
    gFormat.setFontWeight(QFont::Bold);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(G\d+)"),
        GCommand,
        gFormat
    });

    // m commands
    QTextCharFormat mFormat;
    mFormat.setForeground(Qt::darkMagenta);
    mFormat.setFontWeight(QFont::Bold);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(M\d+)"),
        MCommand,
        mFormat
    });

    // variables
    QTextCharFormat variableFormat;
    variableFormat.setForeground(Qt::darkYellow);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(#\d+)"),
        Variable,
        variableFormat
    });

    // positions - alpha component
    QTextCharFormat positionAlphaFormat;
    positionAlphaFormat.setFontWeight(QFont::Bold);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(([XYZABC]{1})(?=([\d\.-])*))"),
        PositionAlpha,
        positionAlphaFormat
    });

    // positions - numeric component
    QTextCharFormat positionNumericFormat;
    positionNumericFormat.setForeground(Qt::darkMagenta);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"((?<=[XYZABC])([\d\.-])*)"),
        PositionNumeric,
        positionNumericFormat
    });

    // arc positions
    QTextCharFormat arcPositionFormat;
    arcPositionFormat.setFontItalic(true);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(([IJK])(?=([\d\.-])*))"),
        ArcPosition,
        arcPositionFormat
    });

    // feeds & speeds
    QTextCharFormat feedSpeedFormat;
    feedSpeedFormat.setForeground(Qt::darkYellow);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"((?<= )[FS]([\d\.-])+)"),
        FeedSpeed,
        feedSpeedFormat
    });

    // O & N numbers
    QTextCharFormat onNumberFormat;
    onNumberFormat.setForeground(Qt::darkCyan);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"(([ON]([\d\.-])+))"),
        OnNumber,
        onNumberFormat
    });

    // brackets
    QTextCharFormat bracketFormat;
    bracketFormat.setForeground(Qt::darkGray);
    m_rules.append(HighlightingRule{
        QRegularExpression(R"((\[|\]))"),
        Bracket,
        bracketFormat
    });

}

// patterns:
// - comment: g commands
//   name: support.variable
//   match: G\d+

// - comment: m commands
//   name: support.constant
//   match: M\d+

// - comment: comments
//   name: comment.line
//   match: \(.*\)

// - comment: flow control operators
//   name: keyword.control
//   match: (IF|THEN|AND|OR|WHILE|GOTO)

// - comment: comparison operators
//   name: keyword.operator
//   match: (GT|LT|GE|LE|EQ)

// - comment: flow control operators - locations
//   name: markup.italic
//   match: (?<=GOTO|DO)\d+

// - comment: functions
//   name: support.function
//   match: (ABS|ACOS|ASIN|ATAN|COS|LN|EXP|FIX|FUP|MOD|ROUND|SQRT|SIN|TAN)

// - comment: variables
//   name: variable.other
//   match: (#\d+)

// - comment: positions - alpha component
//   name: markup.bold
//   match: ([XYZABC]{1})(?=([\d\.-])*)

// - comment: positions - numeric component
//   name: constant.numeric
//   match: (?<=[XYZABC])([\d\.-])*

// - comment: arc positions
//   name: markup.italic
//   match: ([IJK])(?=([\d\.-])*)

// - comment: feeds & speeds
//   name: variable.parameter
//   match: (?<= )[FS]([\d\.-])+

// - comment: O & N numberes
//   name: support.function
//   match: ([ON]([\d\.-])+)

// - comment: brackets
//   name: string.interpolated
//   match: (\[|\])

SyntaxHighlighter::~SyntaxHighlighter()
{
}

QColor SyntaxHighlighter::colorByType(SyntaxHighlighter::ElementType type)
{
    switch (type) {
    case GCommand:
        return QColor(Qt::blue);
    case MCommand:
        return QColor(Qt::darkMagenta);
    case Comment:
        return QColor(Qt::darkGreen);
    case FlowControl:
        return QColor(Qt::darkBlue);
    case Comparison:
        return QColor(Qt::darkRed);
    case Location:
        return QColor(Qt::darkCyan);
    case Function:
        return QColor(Qt::darkCyan);
    case Variable:
        return QColor(Qt::darkYellow);
    case PositionAlpha:
        return QColor(Qt::black);
    case PositionNumeric:
        return QColor(Qt::darkMagenta);
    case ArcPosition:
        return QColor(Qt::darkGray);
    case FeedSpeed:
        return QColor(Qt::darkYellow);
    case OnNumber:
        return QColor(Qt::darkCyan);
    case Bracket:
        return QColor(Qt::darkGray);
    default:
        return {};  // not found
    }
}

void SyntaxHighlighter::colorChanged(ElementType type, const QColor& col)
{
    Q_UNUSED(type);
    Q_UNUSED(col);
    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    QString curText = text.toUpper();
    foreach (const HighlightingRule &rule, m_rules)
    {
        QRegularExpression expression(rule.pattern);

        QRegularExpressionMatchIterator i = expression.globalMatch(curText);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            // replace matched text with spaces to avoid overlapping matches
            curText.replace(match.capturedStart(), match.capturedLength(), QString(match.capturedLength(), ' '));
        }
    }
    setCurrentBlockState(0);
}

void SyntaxHighlighter::setColor(ElementType type, const QColor &color)
{
    for (HighlightingRule &rule : m_rules)
    {
        if (rule.type == type)
        {
            rule.format.setForeground(color);
            break;
        }
    }

    colorChanged(type, color);
}
