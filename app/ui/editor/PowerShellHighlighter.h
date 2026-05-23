#ifndef POWERSHELL_HIGHLIGHTER_H
#define POWERSHELL_HIGHLIGHTER_H

#include "Highlighter.h"

namespace buraq {

class PowerShellHighlighter : public Highlighter {
public:
    PowerShellHighlighter();
    QString highlight(const QString& text) override;

private:
    std::vector<HighlightRule> m_rules;
    QString escapeHtml(const QString& text);
};

} // namespace buraq

#endif // POWERSHELL_HIGHLIGHTER_H
