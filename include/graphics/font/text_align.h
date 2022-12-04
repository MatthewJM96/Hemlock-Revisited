#ifndef __hemlock_graphics_font_text_align_h
#define __hemlock_graphics_font_text_align_h

namespace hemlock {
    namespace graphics {
        namespace font {
            /**
             * @brief This enum is of the various ways in which text may be aligned
             * within a rectangle.
             */
            enum class TextAlign {
                NONE,
                CENTER_LEFT,
                TOP_LEFT,
                TOP_CENTER,
                TOP_RIGHT,
                CENTER_RIGHT,
                BOTTOM_RIGHT,
                BOTTOM_CENTER,
                BOTTOM_LEFT,
                CENTER_CENTER
            };

            /**
             * @brief Calculates the offsets needed for a specific line of text.
             *
             * @param align The alignment of the text.
             * @param rect The rectangle in which the text will be drawn.
             * @param height The height of the entire body of text to be drawn.
             * @param length The length of the line for which the offset is being
             * calculated.
             *
             * @return The offset calculated.
             */
            f32v2 calculate_align_offset(
                TextAlign align, f32v4 rect, f32 height, f32 length
            );
        }  // namespace font
        namespace f = font;
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_font_text_align_h
