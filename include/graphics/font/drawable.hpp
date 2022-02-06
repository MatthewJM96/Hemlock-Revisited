#ifndef __hemlock_graphics_font_drawable_hpp
#define __hemlock_graphics_font_drawable_hpp

#include "graphics/font/instance.hpp"

namespace hemlock {
    namespace graphics {
        namespace font {
            /**
             * @brief Whether the string should be sized (vertically) by a scale factor or target a fixed pixel height.
             */
            enum class StringSizingKind {
                SCALED,
                FIXED
            };

            /**
             * @brief The properties defining the sizing.
             */
            struct StringSizing {
                StringSizingKind kind;
                union {
                    f32v2 scaling;
                    struct {
                        f32 scaleX;
                        f32 target_height;
                    };
                };
            };

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
            using DrawableStringComponents = std::vector<DrawableStringComponent>;
        }
        namespace f = font;
    }
}
namespace hg = hemlock::graphics;

#endif // __hemlock_graphics_font_drawable_hpp
