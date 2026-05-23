#ifndef CPP_HIGHLIGHTER_H
#define CPP_HIGHLIGHTER_H

#include "Highlighter.h"

namespace buraq {

class CppHighlighter : public Highlighter {
public:
    CppHighlighter();
    QString highlight(const QString& text) override;

private:
    std::vector<HighlightRule> m_rules;
    QString escapeHtml(const QString& text);
};

} // namespace buraq

#endif // CPP_HIGHLIGHTER_H
