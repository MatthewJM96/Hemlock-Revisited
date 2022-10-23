#ifndef __hemlock_graphics_font_drawable_hpp
#define __hemlock_graphics_font_drawable_hpp

#include "graphics/font/instance.hpp"

/**
 * @brief Properties defining string sizing, with two different sizing
 * methods, either the string should be sized (vertically) by:
 *  * a scale factor, or
 * * target a fixed pixel height.
 */
H_DEF_UNION_WITH_SERIALISATION(
    hemlock::graphics::font,
    StringSizing,
    ui8,
    (
        SCALED,
        (scaling, f32v2)
    ),
    (
        FIXED,
        (scale_x, f32)
        (target_height, f32)
    )
)

namespace hemlock {
    namespace graphics {
        namespace font {

            /**
             * @brief Properties needed to draw a string.
             */
            struct StringDrawProperties {
                FontInstance font_instance;
                StringSizing sizing;
                colour4      tint;
            };

            /**
             * @brief A pairing of a string with draw properties.
             */
            struct DrawableStringComponent {
                const char*          str;
                StringDrawProperties props;
            };
            using DrawableStringComponents = DrawableStringComponent*;

            /**
             * @brief The data needed to draw a glyph.
             */
            struct DrawableGlyph {
                Glyph*  glyph;
                f32     x_pos;
                f32v2   scaling;
                colour4 tint;
                GLuint  texture;
            };

            /**
             * @brief The data needed to draw a line.
             */
            struct DrawableLine {
                f32 length;
                f32 height;
                std::vector<DrawableGlyph> drawables;
            };
            using DrawableLines = std::vector<DrawableLine>;

            /**
             * @brief This enum is of the various ways in which text may be wrapped.
             *
             * QUICK is the quickest of the wrap modes, where it breaks immediately before the first character to exceed the rectangle.
             *
             * GREEDY only breaks on whitespace or '\\n' characters. While quicker than MINIMUM_RAGGEDNESS, it has the less uniformity of line lengths.
             *
             * MINIMUM_RAGGEDNESS breaks on the same characters as GREEDY but seeks to minimise the difference in line lengths rather than time taken
             *     to calculate.
             */
            enum class WordWrap {
                NONE,
                QUICK,
                GREEDY,
                MINIMUM_RAGGEDNESS
            };
        }
        namespace f = font;
    }
}
namespace hg = hemlock::graphics;

#endif // __hemlock_graphics_font_drawable_hpp
