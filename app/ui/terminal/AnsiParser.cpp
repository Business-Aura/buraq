// AnsiParser.cpp — VT100/ANSI SGR parser implementation
#include "AnsiParser.h"

#include <QRegularExpression>

// --------------------------------------------------------------------------
// Static helpers
// --------------------------------------------------------------------------

static const QColor kStandardColors[8] = {
    QColor(0,   0,   0),    // 0 Black
    QColor(170, 0,   0),    // 1 Red
    QColor(0,   170, 0),    // 2 Green
    QColor(170, 170, 0),    // 3 Yellow
    QColor(0,   0,   170),  // 4 Blue
    QColor(170, 0,   170),  // 5 Magenta
    QColor(0,   170, 170),  // 6 Cyan
    QColor(170, 170, 170),  // 7 White
};

static const QColor kBrightColors[8] = {
    QColor(85,  85,  85),   // 0 Bright Black (Dark Gray)
    QColor(255, 85,  85),   // 1 Bright Red
    QColor(85,  255, 85),   // 2 Bright Green
    QColor(255, 255, 85),   // 3 Bright Yellow
    QColor(85,  85,  255),  // 4 Bright Blue
    QColor(255, 85,  255),  // 5 Bright Magenta
    QColor(85,  255, 255),  // 6 Bright Cyan
    QColor(255, 255, 255),  // 7 Bright White
};

QColor AnsiParser::ansiColorIndex(int index, bool bright)
{
    if (index < 0 || index > 7) return {};
    return bright ? kBrightColors[index] : kStandardColors[index];
}

QColor AnsiParser::ansi256Color(int index)
{
    if (index < 0 || index > 255) return {};

    // 0-7: standard colours
    if (index < 8)  return kStandardColors[index];
    // 8-15: bright colours
    if (index < 16) return kBrightColors[index - 8];

    // 16-231: 6×6×6 colour cube
    if (index < 232)
    {
        int i = index - 16;
        int b = i % 6;
        int g = (i / 6) % 6;
        int r = i / 36;
        auto c = [](int v) { return v == 0 ? 0 : 55 + v * 40; };
        return QColor(c(r), c(g), c(b));
    }

    // 232-255: greyscale ramp
    int v = (index - 232) * 10 + 8;
    return QColor(v, v, v);
}

// --------------------------------------------------------------------------
// AnsiParser
// --------------------------------------------------------------------------

AnsiParser::AnsiParser()
{
    reset();
}

void AnsiParser::reset()
{
    m_fg       = QColor(); // invalid → use theme default
    m_bg       = QColor();
    m_bold     = false;
    m_italic   = false;
    m_underline= false;
    m_dim      = false;
}

void AnsiParser::applySGR(const QList<int>& params)
{
    if (params.isEmpty())
    {
        reset();
        return;
    }

    for (int i = 0; i < params.size(); ++i)
    {
        int p = params[i];
        switch (p)
        {
        case 0:  reset();           break;
        case 1:  m_bold      = true; break;
        case 2:  m_dim       = true; break;
        case 3:  m_italic    = true; break;
        case 4:  m_underline = true; break;
        case 22: m_bold = m_dim = false; break;
        case 23: m_italic    = false; break;
        case 24: m_underline = false; break;

        // Standard foreground (30-37), bright foreground (90-97)
        case 30: case 31: case 32: case 33:
        case 34: case 35: case 36: case 37:
            m_fg = ansiColorIndex(p - 30); break;
        case 39: m_fg = QColor(); break;   // default fg
        case 90: case 91: case 92: case 93:
        case 94: case 95: case 96: case 97:
            m_fg = ansiColorIndex(p - 90, true); break;

        // Standard background (40-47), bright background (100-107)
        case 40: case 41: case 42: case 43:
        case 44: case 45: case 46: case 47:
            m_bg = ansiColorIndex(p - 40); break;
        case 49: m_bg = QColor(); break;   // default bg
        case 100: case 101: case 102: case 103:
        case 104: case 105: case 106: case 107:
            m_bg = ansiColorIndex(p - 100, true); break;

        // 256-color / 24-bit: ESC[38;5;n or ESC[38;2;r;g;b
        case 38:
        case 48:
        {
            if (i + 1 < params.size())
            {
                int sub = params[i + 1];
                if (sub == 5 && i + 2 < params.size())
                {
                    QColor c = ansi256Color(params[i + 2]);
                    if (p == 38) m_fg = c; else m_bg = c;
                    i += 2;
                }
                else if (sub == 2 && i + 4 < params.size())
                {
                    QColor c(params[i+2], params[i+3], params[i+4]);
                    if (p == 38) m_fg = c; else m_bg = c;
                    i += 4;
                }
            }
            break;
        }

        default: break;
        }
    }
}

QList<AnsiSegment> AnsiParser::parse(const QString& input)
{
    QList<AnsiSegment> result;

    // Regex for CSI sequences: ESC [ ... (final byte in 0x40-0x7E)
    static const QRegularExpression re(
        QStringLiteral("\x1b(?:"
            "\\[([0-9;?]*)([A-Za-z])"  // CSI sequence
            "|[()][AB012]"              // Charset designation (ignore)
            "|[ABCDHIJKMPSTX78=><]"     // C1 / 2-char ESC sequences (ignore)
        ")")
    );

    int pos = 0;
    auto it = re.globalMatch(input);

    while (it.hasNext())
    {
        auto match = it.next();
        int start  = match.capturedStart();

        // Emit text before this escape sequence
        if (start > pos)
        {
            QString text = input.mid(pos, start - pos);
            // Filter bare \r (carriage return without newline → move to col 0)
            text.remove('\r');
            if (!text.isEmpty())
            {
                AnsiSegment seg;
                seg.text      = text;
                seg.foreground= m_fg;
                seg.background= m_bg;
                seg.bold      = m_bold;
                seg.italic    = m_italic;
                seg.underline = m_underline;
                seg.dim       = m_dim;
                result.append(seg);
            }
        }

        // Handle CSI sequences
        QString finalByte = match.captured(2);
        if (finalByte == QStringLiteral("m"))
        {
            // SGR — parse parameters
            QString paramStr = match.captured(1);
            QList<int> params;
            if (paramStr.isEmpty())
            {
                params.append(0);
            }
            else
            {
                for (const QString& s : paramStr.split(';'))
                    params.append(s.isEmpty() ? 0 : s.toInt());
            }
            applySGR(params);
        }
        // Other CSI sequences (cursor movement etc.) are consumed / ignored

        pos = match.capturedEnd();
    }

    // Trailing text after last escape sequence
    if (pos < input.size())
    {
        QString text = input.mid(pos);
        text.remove('\r');
        if (!text.isEmpty())
        {
            AnsiSegment seg;
            seg.text      = text;
            seg.foreground= m_fg;
            seg.background= m_bg;
            seg.bold      = m_bold;
            seg.italic    = m_italic;
            seg.underline = m_underline;
            seg.dim       = m_dim;
            result.append(seg);
        }
    }

    return result;
}
