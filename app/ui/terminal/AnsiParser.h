// AnsiParser.h — VT100/ANSI escape sequence parser
#pragma once

#include <QColor>
#include <QList>
#include <QString>

// A single styled text run produced by the ANSI parser
struct AnsiSegment
{
    QString text;
    QColor  foreground;   // invalid = use theme default
    QColor  background;   // invalid = use theme default
    bool    bold      = false;
    bool    italic    = false;
    bool    underline = false;
    bool    dim       = false;
};

class AnsiParser
{
public:
    AnsiParser();

    // Parse `input` (may contain ANSI/VT100 escape sequences).
    // Returns a list of styled segments in order.
    QList<AnsiSegment> parse(const QString& input);

    // Reset all graphic-rendition state to defaults
    void reset();

private:
    // --- current SGR state ---
    QColor m_fg;          // invalid → default foreground
    QColor m_bg;          // invalid → default background
    bool   m_bold      = false;
    bool   m_italic    = false;
    bool   m_underline = false;
    bool   m_dim       = false;

    void applySGR(const QList<int>& params);

    // 3-bit (standard) & 8-bit colour mapping
    static QColor ansiColorIndex(int index, bool bright = false);
    static QColor ansi256Color(int index);
};
