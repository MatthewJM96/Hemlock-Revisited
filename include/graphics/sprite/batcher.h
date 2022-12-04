#ifndef __hemlock_graphics_sprite_batcher_h
#define __hemlock_graphics_sprite_batcher_h

#include "graphics/font/drawable.hpp"
#include "graphics/font/text_align.h"
#include "graphics/glsl_program.h"
#include "graphics/sprite/sprite.hpp"

namespace hemlock {
    namespace graphics {
        // Forward declarations
        namespace font {
            class FontCache;
        }

        namespace sprite {
            /**
             * @brief Implementation of sprite batching. Sprites are drawn after
             * the sprite batch phase begins, after the end of which they are sorted
             * and their vertex data is collated and sent to the GPU ready for
             * rendering.
             */
            class SpriteBatcher {
                using Sprites    = std::vector<Sprite>;
                using SpritePtrs = std::vector<Sprite*>;
                using Batches    = std::vector<SpriteBatch>;
            public:
                SpriteBatcher();

                ~SpriteBatcher() { /* Empty. */
                }

                /**
                 * @brief Initialises the sprite batcher, setting the shader
                 * cache, an optional font cache, and buffer mode.
                 *
                 * @param shader_cache Shader cache.
                 * @param font_cache Optional font cache.
                 * @param is_dynamic Whether buffer should be managed as dynamic
                 * or not.
                 */
                void init(
                    ShaderCache*     shader_cache,
                    font::FontCache* font_cache = nullptr,
                    bool             is_dynamic = true
                );
                /**
                 * @brief Disposes of the sprite batcher.
                 */
                void dispose();

                /**
                 * @brief Reserves count many sprite positions in the sprite buffer,
                 * if this is less than the number of sprites currently stored it does
                 * nothing.
                 */
                void reserve(size_t count);

                /**
                 * @brief Begins the sprite batching phase. Call this BEFORE ANY call
                 * to a "draw" function!
                 */
                void begin();
                /**
                 * @brief Ends the sprite batching phase, the sprites are sorted and
                 * the batches are generated, sending the vertex buffers to the GPU.
                 * Call this AFTER ALL calls to "draw" functions and BEFORE ANY call
                 * to a "render" function.
                 *
                 * @param sort_mode The mode with which to sort sprites. Default is
                 * to sort sprites by texture ID to minimise GPU draw calls.
                 */
                void end(SpriteSortMode sort_mode = SpriteSortMode::TEXTURE);

                /**
                 * @brief Draw the sprite given.
                 *
                 * @param sprite The sprite to draw.
                 */
                void add_sprite(Sprite&& sprite);
                /**
                 * @brief Draw the sprites given.
                 *
                 * @param sprites The sprites to draw.
                 * @param sprite_count The number of sprites passed.
                 */
                void add_sprite(Sprite* sprites, ui32 sprite_count);
                /**
                 * @brief Draw a sprite with the given properties. This
                 * version of draw allows setting every possible aspect
                 * of a sprite's information.
                 *
                 * @param builder The function to use to build a quad.
                 * @param texture The texture of the sprite.
                 * @param position The position of the sprite.
                 * @param size The size of the sprite.
                 * @param c1 The first colour of the sprite.
                 * @param c2 The second colour of the sprite. Only affects the
                 * sprite's rendering if a gradient other than Gradient::NONE
                 * is selected (the default).
                 * @param gradient The gradient of the colours of the sprite.
                 * @param depth The depth of the sprite.
                 * @param uv_rect The normalised UV coordinates and size of the
                 * section of the texture given to use for the sprite (i.e. if
                 * we want to only use a subsection of the texture).
                 */
                void add_sprite(
                    QuadBuilder builder,
                    GLuint      texture,
                    f32v2       position,
                    f32v2       size,
                    colour4     c1       = { 255, 255, 255, 255 },
                    colour4     c2       = { 255, 255, 255, 255 },
                    Gradient    gradient = Gradient::NONE,
                    f32         depth    = 0.0f,
                    f32v4       uv_rect  = f32v4(0.0f, 0.0f, 1.0f, 1.0f)
                );
                /**
                 * @brief Draw a sprite with the given properties. This
                 * version of draw allows setting every possible aspect
                 * of a sprite's information except the texture that is
                 * set to the default.
                 *
                 * @param builder The function to use to build a quad.
                 * @param position The position of the sprite.
                 * @param size The size of the sprite.
                 * @param c1 The first colour of the sprite.
                 * @param c2 The second colour of the sprite. Only affects the
                 * sprite's rendering if a gradient other than Gradient::NONE
                 * is selected (the default).
                 * @param gradient The gradient of the colours of the sprite.
                 * @param depth The depth of the sprite.
                 * @param uv_rect The normalised UV coordinates and size of the
                 * section of the texture given to use for the sprite (i.e. if
                 * we want to only use a subsection of the texture).
                 */
                void add_sprite(
                    QuadBuilder builder,
                    f32v2       position,
                    f32v2       size,
                    colour4     c1       = { 255, 255, 255, 255 },
                    colour4     c2       = { 255, 255, 255, 255 },
                    Gradient    gradient = Gradient::NONE,
                    f32         depth    = 0.0f,
                    f32v4       uv_rect  = f32v4(0.0f, 0.0f, 1.0f, 1.0f)
                );
                /**
                 * @brief Draw a sprite with the given properties. This
                 * version of draw allows setting every possible aspect
                 * of a sprite's information except the quad builder
                 * that is set to the default quad builder function.
                 *
                 * @param texture The texture of the sprite.
                 * @param position The position of the sprite.
                 * @param size The size of the sprite.
                 * @param c1 The first colour of the sprite.
                 * @param c2 The second colour of the sprite. Only affects the
                 * sprite's rendering if a gradient other than Gradient::NONE
                 * is selected (the default).
                 * @param gradient The gradient of the colours of the sprite.
                 * @param depth The depth of the sprite.
                 * @param uv_rect The normalised UV coordinates and size of the
                 * section of the texture given to use for the sprite (i.e. if
                 * we want to only use a subsection of the texture).
                 */
                void add_sprite(
                    GLuint   texture,
                    f32v2    position,
                    f32v2    size,
                    colour4  c1       = { 255, 255, 255, 255 },
                    colour4  c2       = { 255, 255, 255, 255 },
                    Gradient gradient = Gradient::NONE,
                    f32      depth    = 0.0f,
                    f32v4    uv_rect  = f32v4(0.0f, 0.0f, 1.0f, 1.0f)
                );
                /**
                 * @brief Draw a sprite with the given properties. This
                 * version of draw allows setting every possible aspect
                 * of a sprite's information except the texture that is
                 * set to the default, and.the quad builder that is set
                 * to the default quad builder function.
                 *
                 * @param position The position of the sprite.
                 * @param size The size of the sprite.
                 * @param c1 The first colour of the sprite.
                 * @param c2 The second colour of the sprite. Only affects the
                 * sprite's rendering if a gradient other than Gradient::NONE
                 * is selected (the default).
                 * @param gradient The gradient of the colours of the sprite.
                 * @param depth The depth of the sprite.
                 * @param uv_rect The normalised UV coordinates and size of the
                 * section of the texture given to use for the sprite (i.e. if
                 * we want to only use a subsection of the texture).
                 */
                void add_sprite(
                    f32v2    position,
                    f32v2    size,
                    colour4  c1       = { 255, 255, 255, 255 },
                    colour4  c2       = { 255, 255, 255, 255 },
                    Gradient gradient = Gradient::NONE,
                    f32      depth    = 0.0f,
                    f32v4    uv_rect  = f32v4(0.0f, 0.0f, 1.0f, 1.0f)
                );

                // TODO(Matthew): cache does not support aliasing, so font name for
                // now
                //                must be the full filename.
                /**
                 * @brief Draw a string with the given properties.
                 *
                 * @param str The string to draw.
                 * @param target_rect The bounding rectangle the string is aimed to be
                 * kept within.
                 * @param clip_rect The bounding rectangle the string must be kept
                 * within.
                 * @param sizing The sizing of the font.
                 * @param tint The colour to give the string.
                 * @param font_name The name of the font to use.
                 * @param font_size The size of the font to use.
                 * @param align The alignment to use for the string.
                 * @param wrap The wrapping mode to use for the string.
                 * @param depth The depth of the string for rendering.
                 * @param style The style of the font to use.
                 * @param render_style The rendering style to use for the font.
                 */
                void add_string(
                    CALLER_DELETE const char* str,
                    f32v4                     target_rect,
                    f32v4                     clip_rect,
                    f::StringSizing           sizing,
                    colour4                   tint,
                    const std::string&        font_name,
                    f::FontSize               font_size,
                    f::TextAlign              align        = f::TextAlign::TOP_LEFT,
                    f::WordWrap               wrap         = f::WordWrap::NONE,
                    f32                       depth        = 0.0f,
                    f::FontStyle              style        = f::FontStyle::NORMAL,
                    f::FontRenderStyle        render_style = f::FontRenderStyle::BLENDED
                );
                /**
                 * @brief Draw a string with the given properties. Font
                 * size is taken to be the default for the font specified.
                 *
                 * @param str The string to draw.
                 * @param target_rect The bounding rectangle the string is aimed to be
                 * kept within.
                 * @param clip_rect The bounding rectangle the string must be kept
                 * within.
                 * @param sizing The sizing of the font.
                 * @param tint The colour to give the string.
                 * @param font_name The name of the font to use.
                 * @param align The alignment to use for the string.
                 * @param wrap The wrapping mode to use for the string.
                 * @param depth The depth of the string for rendering.
                 * @param style The style of the font to use.
                 * @param render_style The rendering style to use for the font.
                 */
                void add_string(
                    CALLER_DELETE const char* str,
                    f32v4                     target_rect,
                    f32v4                     clip_rect,
                    f::StringSizing           sizing,
                    colour4                   tint,
                    const std::string&        font_name,
                    f::TextAlign              align        = f::TextAlign::TOP_LEFT,
                    f::WordWrap               wrap         = f::WordWrap::NONE,
                    f32                       depth        = 0.0f,
                    f::FontStyle              style        = f::FontStyle::NORMAL,
                    f::FontRenderStyle        render_style = f::FontRenderStyle::BLENDED
                );
                /**
                 * @brief Draw a string with the given properties. Font
                 * instance is explicitly specified in this version.
                 *
                 * @param str The string to draw.
                 * @param target_rect The bounding rectangle the string is aimed to be
                 * kept within.
                 * @param clip_rect The bounding rectangle the string must be kept
                 * within.
                 * @param sizing The sizing of the font.
                 * @param tint The colour to give the string.
                 * @param font_instance The instance of the font to use.
                 * @param align The alignment to use for the string.
                 * @param wrap The wrapping mode to use for the string.
                 * @param depth The depth of the string for rendering.
                 */
                void add_string(
                    CALLER_DELETE const char* str,
                    f32v4                     target_rect,
                    f32v4                     clip_rect,
                    f::StringSizing           sizing,
                    colour4                   tint,
                    f::FontInstance           font_instance,
                    f::TextAlign              align = f::TextAlign::TOP_LEFT,
                    f::WordWrap               wrap  = f::WordWrap::NONE,
                    f32                       depth = 0.0f
                );
                /**
                 * @brief Draw a string with the given properties. Font
                 * instance is explicitly specified in this version.
                 *
                 * @param str_component The string to draw with drawable properties.
                 * @param target_rect The bounding rectangle the string is aimed to be
                 * kept within.
                 * @param clip_rect The bounding rectangle the string must be kept
                 * within.
                 * @param align The alignment to use for the string.
                 * @param wrap The wrapping mode to use for the string.
                 * @param depth The depth of the string for rendering.
                 */
                void add_string(
                    CALLER_DELETE f::DrawableStringComponent str_component,
                    f32v4                                    target_rect,
                    f32v4                                    clip_rect,
                    f::TextAlign align = f::TextAlign::TOP_LEFT,
                    f::WordWrap  wrap  = f::WordWrap::NONE,
                    f32          depth = 0.0f
                );
                /**
                 * @brief Draw a string with the given properties. This version is
                 * the most flexible method for drawing strings, enabling multiple
                 * components, each its own sub-string possessing its own properties.
                 *
                 * @param str_components The components of the string, consisting of
                 * sub-strings and their properties.
                 * @param num_components The number of components to be added.
                 * @param target_rect The bounding rectangle the string is aimed to be
                 * kept within.
                 * @param clip_rect The bounding rectangle the string must be kept
                 * within.
                 * @param align The alignment to use for the string.
                 * @param wrap The wrapping mode to use for the string.
                 * @param depth The depth of the string for rendering.
                 */
                void add_string(
                    CALLER_DELETE f::DrawableStringComponents str_components,
                    ui32                                      num_components,
                    f32v4                                     target_rect,
                    f32v4                                     clip_rect,
                    f::TextAlign align = f::TextAlign::TOP_LEFT,
                    f::WordWrap  wrap  = f::WordWrap::NONE,
                    f32          depth = 0.0f
                );

                /**
                 * @brief Sets the shader to be used by the sprite batcher. If the
                 * shader that is passed in is unlinked, it is assumed the attributes
                 * are to be set as the defaults and so they are set as such and the
                 * shader linked.
                 *
                 * @param shader The shader to use. If this is nullptr, then the
                 * default shader is set as the active shader.
                 *
                 * @return True if the shader was successfully set, false otherwise.
                 */
                bool set_shader(GLSLProgram* shader = nullptr);

                /**
                 * @brief Render the batches that have been generated.
                 *
                 * @param world_projection The projection matrix to go from world
                 * coords to "camera" coords.
                 * @param view_projection The projection matrix to go from "camera"
                 * coords to screen coords.
                 */
                void render(f32m4 world_projection, f32m4 view_projection);
                /**
                 * @brief Render the batches that have been generated.
                 *
                 * This method is useful if you just want to draw to the screen with
                 * some sense of the sprites being 2D in the world (e.g. a marker
                 * above an NPCs head).
                 *
                 * @param world_projection The projection matrix to go from world
                 * coords to "camera" coords.
                 * @param screen_size The size of the screen.
                 */
                void render(f32m4 world_projection, f32v2 screen_size);
                /**
                 * @brief Render the batches that have been generated.
                 *
                 * This method is useful if you just want to draw to the screen
                 * without placing the sprites in the world at all (e.g. UI elements
                 * like the main menu).
                 *
                 * @param screen_size The size of the screen.
                 */
                void render(f32v2 screen_size);
            protected:
                /**
                 * @brief Sorts the sprites using the given sort mode.
                 *
                 * @param sort_mode The mode by which to sort the sprites.
                 */
                void sort_sprites(SpriteSortMode sort_mode);

                /**
                 * @brief Generates batches from the drawn sprites.
                 */
                void generate_batches();

                bool m_is_initialised;

                Sprites    m_sprites;
                SpritePtrs m_sprite_ptrs;
                Batches    m_batches;

                GLuint m_vao, m_vbo, m_ibo;
                GLenum m_usage_hint;
                ui32   m_index_count;

                ui32        m_default_texture;
                GLSLProgram m_default_shader;

                GLSLProgram* m_active_shader;

                ShaderCache*     m_shader_cache;
                font::FontCache* m_font_cache;
            };

            namespace impl {
                /**
                 * @brief Create a quad from a sprite.
                 *
                 * @param sprite The sprite to create the quad from.
                 * @param vertices The vertices resulting from the build.
                 */
                void basic_build_quad(const Sprite* sprite, SpriteVertex* vertices);

                /**
                 * @brief Compares sprites by texture ID for sorting (such that all
                 * sprites with the same texture will be in one batch together - this
                 * is the quickest for rendering by generating the least batches).
                 *
                 * @param sprite1 The first sprite to compare.
                 * @param sprite2 The second sprite to compare.
                 */
                bool sort_texture(Sprite* sprite1, Sprite* sprite2);
                /**
                 * @brief Compares sprites by depth for sorting (such that sprites
                 * with less depth are ordered first).
                 *
                 * @param sprite1 The first sprite to compare.
                 * @param sprite2 The second sprite to compare.
                 */
                bool sort_front_to_back(Sprite* sprite1, Sprite* sprite2);
                /**
                 * @brief Compares sprites by depth for sorting (such that sprites
                 * with greater depth are ordered first).
                 *
                 * @param sprite1 The first sprite to compare.
                 * @param sprite2 The second sprite to compare.
                 */
                bool sort_back_to_front(Sprite* sprite1, Sprite* sprite2);
            }  // namespace impl
        }      // namespace sprite
        namespace s = sprite;
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_sprite_batcher_h
