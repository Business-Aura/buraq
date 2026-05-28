#include "PowerShellHighlighter.h"

PowerShellHighlighter::PowerShellHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    updateTheme(ThemeManager::instance().currentTheme());
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &PowerShellHighlighter::updateTheme);
}

void PowerShellHighlighter::updateTheme(AppTheme theme)
{
    highlightingRules.clear();

    HighlightingRule rule;

    keywordFormat.setForeground((theme == Dark) ? QColor("#CC7832") : QColor("#0033B3")); // Darcula/IntelliJ Light Keyword
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bif\\b"), QStringLiteral("\\belse\\b"), QStringLiteral("\\belseif\\b"),
        QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bforeach\\b"), QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bdo\\b"), QStringLiteral("\\buntil\\b"),
        QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bcontinue\\b"), QStringLiteral("\\breturn\\b"),
        QStringLiteral("\\bfunction\\b"), QStringLiteral("\\bfilter\\b"), QStringLiteral("\\bglobal\\b"),
        QStringLiteral("\\bscript\\b"), QStringLiteral("\\blocal\\b"), QStringLiteral("\\bprivate\\b"),
        QStringLiteral("\\bparam\\b"), QStringLiteral("\\bbegin\\b"), QStringLiteral("\\bprocess\\b"),
        QStringLiteral("\\bend\\b"), QStringLiteral("\\bdynamicparam\\b"), QStringLiteral("\\bclass\\b"),
        QStringLiteral("\\benum\\b"), QStringLiteral("\\bhidden\\b"), QStringLiteral("\\bstatic\\b")
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    variableFormat.setForeground((theme == Dark) ? QColor("#9876AA") : QColor("#000000")); // Darcula/IntelliJ Light Variable
    rule.pattern = QRegularExpression(QStringLiteral("\\$[A-Za-z0-9_]+"));
    rule.format = variableFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground((theme == Dark) ? QColor("#808080") : QColor("#8C8C8C")); // Darcula/IntelliJ Light Comment
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground((theme == Dark) ? QColor("#808080") : QColor("#8C8C8C")); // Darcula/IntelliJ Light Comment

    stringFormat.setForeground((theme == Dark) ? QColor("#6A8759") : QColor("#067D17")); // Darcula/IntelliJ Light String
    rule.pattern = QRegularExpression(QStringLiteral("(\"[^\"]*\")|('[^']*')"));
    rule.format = stringFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground((theme == Dark) ? QColor("#FFC66D") : QColor("#00627A")); // Darcula/IntelliJ Light Function
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+-[A-Za-z0-9_]+\\b"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression(QStringLiteral("<#"));
    commentEndExpression = QRegularExpression(QStringLiteral("#>"));

    rehighlight();
}

void PowerShellHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
