#ifndef __hemlock_graphics_sprite_sprite_hpp
#define __hemlock_graphics_sprite_sprite_hpp

#include "graphics/gradient.hpp"

namespace hemlock {
    namespace graphics {
        namespace sprite {
            // Forward declarations.
            struct Sprite;
            struct SpriteVertex;

            /**
             * @brief Defines invokable type for building a quad from a sprite and
             * vertex information.
             */
            using QuadBuilder
                = Delegate<void(const Sprite* sprite, SpriteVertex* vertices)>;

            /**
             * @brief The sorting modes allowed for sorting sprites.
             */
            enum class SpriteSortMode {
                BACK_TO_FRONT,
                FRONT_TO_BACK,
                TEXTURE
            };

            /**
             * @brief The properties that define a sprite.
             */
            struct Sprite {
                QuadBuilder build;
                GLuint      texture;
                f32v2       position;
                f32v2       size;
                f32         depth;
                f32v4       uv_rect;
                colour4     c1, c2;
                Gradient    gradient;
                // TODO(Matthew): Offsets, rotations, etc?
                // TODO(Matthew): Custom gradients? Different blending styles?
            };

            /**
             * @brief The properties that define a batch - where a
             * batch is a collection of sprites with the same texture
             * that are consecutively positioned within the SpriteBatcher.
             */
            struct SpriteBatch {
                GLuint texture;
                ui32   index_count;
                ui32   index_offset;
            };

            /**
             * @brief The properties of a vertex of a sprite. We use this to
             * build up the array of data we need to send to the GPU for rendering.
             */
            struct SpriteVertex {
                f32v3   position;
                f32v2   relative_position;
                f32v4   uv_rect;
                colour4 colour;
            };

            /**
             * @brief A set of shader attribute IDs we use for setting and linking
             * variables in our shaders to the data we send to the GPU. (Note how
             * they correspond to the SpriteVertex properties.)
             */
            enum SpriteShaderAttribID : GLuint {
                POSITION = 0,
                RELATIVE_POSITION,
                UV_DIMENSIONS,
                COLOUR,
                SENTINEL
            };
        }  // namespace sprite
        namespace s = sprite;
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_sprite_sprite_hpp
