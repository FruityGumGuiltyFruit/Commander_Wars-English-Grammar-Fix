#pragma once
#include "3rd_party/oxygine-framework/oxygine/oxygine-forwards.h"
#include "3rd_party/oxygine-framework/oxygine/core/intrusive_ptr.h"
#include "3rd_party/oxygine-framework/oxygine/math/Rect.h"
#include <QSet>

namespace oxygine
{
    struct glyph;
    inline quint32 qHash(const oxygine::glyph& k, quint32 seed = 0) noexcept;

    struct glyph
    {
        RectF src;

        qint32 ch{0};

        short sw{0};
        short sh{0};

        short offset_x{0};
        short offset_y{0};

        short advance_x{0};
        short advance_y{0};

        spTexture texture;

        bool operator == (const glyph& r) const
        {
            return ch == r.ch;
        }
    };

    class Font;
    using spFont = oxygine::intrusive_ptr<Font>;
    class Font : public ref_counter
    {
    public:
        explicit Font() = default;
        virtual ~Font() = default;

        void init(qint32 size, qint32 baselineDistance);

        void addGlyph(const glyph& g);
        void sortGlyphs() {}

        void clear();

        void setScale(float scale)
        {
            m_scale = scale;
        }
        void setBaselineDistance(qint32 d)
        {
            m_baselineDistance = d;
        }

        const glyph*    getGlyph(qint32 code) const;
        qint32          getBaselineDistance() const;
        qint32          getSize() const;
        float           getScale() const;

    protected:
        const glyph* findGlyph(qint32 code) const;
        virtual bool loadGlyph(int, glyph&)
        {
            return false;
        }
    protected:
        using glyphs = QSet<glyph>;
        glyphs m_glyphs;
        float m_scale{1.0f};
        qint32 m_size{0};
        qint32 m_baselineDistance{0};
    };
}
