#ifndef POWERSHELL_HIGHLIGHTER_H
#define POWERSHELL_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "Filters/ThemeManager/ThemeManager.h"

class PowerShellHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    PowerShellHighlighter(QTextDocument *parent = nullptr);

public slots:
    void updateTheme(AppTheme theme, bool triggerRehighlight = true);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat variableFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat functionFormat;
};

#endif // POWERSHELL_HIGHLIGHTER_H
