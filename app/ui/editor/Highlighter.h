#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QString>
#include <QRegularExpression>
#include <vector>

namespace buraq {

struct HighlightRule {
    QRegularExpression pattern;
    QString style; // CSS style or color
};

class Highlighter {
public:
    virtual ~Highlighter() = default;
    virtual QString highlight(const QString& text) = 0;
};

} // namespace buraq

#endif // HIGHLIGHTER_H
