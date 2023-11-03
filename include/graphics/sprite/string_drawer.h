#ifndef __hemlock_graphics_sprite_string_drawer_h
#define __hemlock_graphics_sprite_string_drawer_h

#include "graphics/font/drawable.h"
#include "graphics/font/text_align.h"

namespace hemlock {
    namespace graphics {
        namespace sprite {
            class SpriteBatcher;
        }

        /******************************************************\
         * No Wrap Draw                                       *
        \******************************************************/

        /**
         * @brief Adds a string with no wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param component The string plus properties to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        void add_string_no_wrap(
            sprite::SpriteBatcher*     batcher,
            f::DrawableStringComponent component,
            f32v4                      target_rect,
            f32v4                      clip_rect,
            f::TextAlign               align,
            f32                        depth
        );
        /**
         * @brief Adds a string with no wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param components The string components to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        void add_string_no_wrap(
            sprite::SpriteBatcher*      batcher,
            f::DrawableStringComponents components,
            ui32                        num_components,
            f32v4                       target_rect,
            f32v4                       clip_rect,
            f::TextAlign                align,
            f32                         depth
        );

        /******************************************************\
         * Quick Wrap Draw                                    *
        \******************************************************/

        /**
         * @brief Adds a string with quick wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param component The string plus properties to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        void add_string_quick_wrap(
            sprite::SpriteBatcher*     batcher,
            f::DrawableStringComponent component,
            f32v4                      target_rect,
            f32v4                      clip_rect,
            f::TextAlign               align,
            f32                        depth
        );
        /**
         * @brief Adds a string with quick wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param components The string components to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        void add_string_quick_wrap(
            sprite::SpriteBatcher*      batcher,
            f::DrawableStringComponents components,
            ui32                        num_components,
            f32v4                       target_rect,
            f32v4                       clip_rect,
            f::TextAlign                align,
            f32                         depth
        );

        /******************************************************\
         * Greedy Wrap Draw                                   *
        \******************************************************/

        /**
         * @brief Adds a string with greedy wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param component The string plus properties to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        void add_string_greedy_wrap(
            sprite::SpriteBatcher*     batcher,
            f::DrawableStringComponent component,
            f32v4                      target_rect,
            f32v4                      clip_rect,
            f::TextAlign               align,
            f32                        depth
        );
        /**
         * @brief Adds a string with greedy wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param components The string components to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        void add_string_greedy_wrap(
            sprite::SpriteBatcher*      batcher,
            f::DrawableStringComponents components,
            ui32                        num_components,
            f32v4                       target_rect,
            f32v4                       clip_rect,
            f::TextAlign                align,
            f32                         depth
        );

        /******************************************************\
         * Minimum Raggedness Wrap Draw                       *
        \******************************************************/

        /**
         * @brief Adds a string with greedy wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param component The string plus properties to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        // void add_string_minimum_raggedness_wrap(sprite::SpriteBatcher* batcher,
        // f::DrawableStringComponent component, f32v4 target_rect, f32v4 clip_rect,
        // f::TextAlign align, f32 depth);
        /**
         * @brief Adds a string with greedy wrapping to the sprite batcher.
         *
         * @param batcher The sprite batcher to draw the string to.
         * @param components The string components to draw.
         * @param target_rect The bounding rectangle the string is aimed to be kept
         * within.
         * @param clip_rect The bounding rectangle the string must be kept within.
         * @param align The alignment for the text.
         * @param depth The depth at which to render the string.
         */
        // void add_string_minimum_raggedness_wrap(sprite::SpriteBatcher* batcher,
        // f::DrawableStringComponents components, ui32 num_components, f32v4
        // target_rect, f32v4 clip_rect, f::TextAlign align, f32 depth);
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_sprite_string_drawer_h
