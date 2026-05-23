#include "PowerShellHighlighter.h"
#include <QTextDocument>

namespace buraq {

PowerShellHighlighter::PowerShellHighlighter() {
    // Keywords
    m_rules.push_back({QRegularExpression("\\b(if|else|elseif|for|foreach|while|do|until|switch|case|default|break|continue|return|try|catch|finally|throw|function|filter|workflow|parallel|sequence|in|process|begin|end|param|exit|using|class|enum|interface|namespace)\\b", QRegularExpression::CaseInsensitiveOption), "color:#FFB76B"});

    // Common Cmdlets
    m_rules.push_back({QRegularExpression("\\b(Get-|Set-|New-|Remove-|Invoke-|Add-|Write-|Test-|Export-|Import-|Enter-|Exit-|Select-|Where-|Sort-|Group-|Measure-|Format-|Out-)[a-zA-Z0-9]+\\b", QRegularExpression::CaseInsensitiveOption), "color:#FFB76B"});

    // Variables
    m_rules.push_back({QRegularExpression("\\$\\w+", QRegularExpression::CaseInsensitiveOption), "color:#87CEEB"});

    // Strings
    m_rules.push_back({QRegularExpression("\"(.*?)\""), "color:#3eb489"});
    m_rules.push_back({QRegularExpression("'(.*?)'"), "color:#3eb489"});

    // Comments
    m_rules.push_back({QRegularExpression("#.*"), "color:gray"});
}

QString PowerShellHighlighter::escapeHtml(const QString& text) {
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#39;");
    return escaped;
}

QString PowerShellHighlighter::highlight(const QString& text) {
    if (text.isEmpty()) {
        return "<p> </p>";
    }

    QString highlighted = escapeHtml(text);

    // This is a simple implementation that doesn't handle overlapping rules perfectly
    // For a production editor, QSyntaxHighlighter is much better.
    // But we are sticking to the existing architecture of HTML conversion.

    // We need to apply rules carefully to avoid highlighting within HTML tags.
    // A simple way is to find all matches, sort them, and then build the HTML.

    struct Match {
        int start;
        int length;
        QString style;
    };
    std::vector<Match> matches;

    for (const auto& rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            matches.push_back({match.capturedStart(), match.capturedLength(), rule.style});
        }
    }

    // Sort matches by start position
    std::sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
        if (a.start != b.start) return a.start < b.start;
        return a.length > b.length; // Longest match first if same start
    });

    QString result;
    int lastPos = 0;
    for (const auto& match : matches) {
        if (match.start < lastPos) continue; // Skip overlapping

        result += escapeHtml(text.mid(lastPos, match.start - lastPos));
        result += "<span style='" + match.style + "'>" + escapeHtml(text.mid(match.start, match.length)) + "</span>";
        lastPos = match.start + match.length;
    }
    result += escapeHtml(text.mid(lastPos));

    return "<p>" + result + "</p>";
}

} // namespace buraq
