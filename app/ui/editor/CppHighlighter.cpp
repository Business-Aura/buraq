#include "CppHighlighter.h"
#include <algorithm>

namespace buraq {

CppHighlighter::CppHighlighter() {
    // Keywords
    m_rules.push_back({QRegularExpression("\\b(alignas|alignof|and|and_eq|asm|atomic_cancel|atomic_commit|atomic_noexcept|auto|bitand|bitor|break|case|catch|class|compl|concept|const|consteval|constexpr|constinit|const_cast|continue|co_await|co_return|co_yield|decltype|default|delete|do|dynamic_cast|else|enum|explicit|export|extern|false|for|friend|goto|if|inline|mutable|namespace|new|noexcept|not|not_eq|nullptr|operator|or|or_eq|private|protected|public|reflexpr|register|reinterpret_cast|requires|return|sizeof|static|static_assert|static_cast|struct|switch|synchronized|template|this|thread_local|throw|true|try|typedef|typeid|typename|union|using|virtual|volatile|while|xor|xor_eq)\\b"), "color:#FFB76B"});

    // Types
    m_rules.push_back({QRegularExpression("\\b(bool|char|char8_t|char16_t|char32_t|double|float|int|long|short|signed|unsigned|void|wchar_t|size_t|int8_t|int16_t|int32_t|int64_t|uint8_t|uint16_t|uint32_t|uint64_t)\\b"), "color:#87CEEB"});

    // Preprocessor directives
    m_rules.push_back({QRegularExpression("^\\s*#\\s*\\w+"), "color:#BD93F9"});

    // Strings
    m_rules.push_back({QRegularExpression("\".*?\""), "color:#3eb489"});
    m_rules.push_back({QRegularExpression("'.*?'"), "color:#3eb489"});

    // Comments
    m_rules.push_back({QRegularExpression("//.*"), "color:gray"});
    m_rules.push_back({QRegularExpression("/\\*.*?\\*/"), "color:gray"});
}

QString CppHighlighter::escapeHtml(const QString& text) {
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#39;");
    return escaped;
}

QString CppHighlighter::highlight(const QString& text) {
    if (text.isEmpty()) {
        return "<p> </p>";
    }

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

    std::sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
        if (a.start != b.start) return a.start < b.start;
        return a.length > b.length;
    });

    QString result;
    int lastPos = 0;
    for (const auto& match : matches) {
        if (match.start < lastPos) continue;

        result += escapeHtml(text.mid(lastPos, match.start - lastPos));
        result += "<span style='" + match.style + "'>" + escapeHtml(text.mid(match.start, match.length)) + "</span>";
        lastPos = match.start + match.length;
    }
    result += escapeHtml(text.mid(lastPos));

    return "<p>" + result + "</p>";
}

} // namespace buraq
