#ifndef __hemlock_graphics_font_instance_hpp
#define __hemlock_graphics_font_instance_hpp

namespace hemlock {
    namespace graphics {
        namespace font {
            const char FIRST_PRINTABLE_CHAR = 32;
            const char LAST_PRINTABLE_CHAR  = 126;

            /**
             * @brief Type used for font size in exposed APIs.
             *
             * Note that this is deliberately smaller than the ui32 used by SDL_ttf as it lets us create unique hashes of 
             * the font render style, font style and font size for unordered map storage.
             */
            using FontSize = ui16;

            /**
             * @brief Enumeration of styles of fonts.
             */
            enum class FontStyle : ui32 {
                BOLD          = TTF_STYLE_BOLD,
                ITALIC        = TTF_STYLE_ITALIC,
                UNDERLINE     = TTF_STYLE_UNDERLINE,
                STRIKETHROUGH = TTF_STYLE_STRIKETHROUGH,
                NORMAL        = TTF_STYLE_NORMAL
            };

            /**
             * @brief Enumeration of styles of font rendering.
             */
            enum class FontRenderStyle : ui8 {
                SOLID, // -> No anti-aliasing, glyph edges will look jagged.
                BLENDED // -> Anti-aliased, glyph edges will look smooth.
            };

            using FontInstanceHash = ui64;
            FontInstanceHash hash(FontSize size, FontStyle style, FontRenderStyle renderStyle);

            /**
             * @brief Data for each glyph.
             */
            struct Glyph {
                char  character;
                f32v4 uvDimensions;
                f32v2 size;
                bool supported;
            };

            // Forward declare Font.
            class Font;

            /**
             * @brief Data for an instance of a font with specific render style, and font style and size.
             *
             * Each font instance consists of a texture which contains each glyph (character) in
             * the font drawn in the size, style and render style specified for the instance. In
             * addition to this texture, it contains a parameter defining the height of the
             * tallest character, as well as an array of metadata for each glyph.
             *     The metadata, Glyph, stores the character, the UV coordinates withing the
             *     texture of the glyph the metadata represents, and the size of the glyph in
             *     pixels.
             */
            struct FontInstance {
                GLuint texture;
                ui32   height;
                Glyph* glyphs;
                Font*  owner;
                ui32v2 texture_size;

                bool save(std::string filepath, hio::image::Saver save);
            };
            const FontInstance NIL_FONT_INSTANCE = { 0, 0, nullptr, nullptr, ui32v2(0) };
        }
        namespace f = font;
    }
}
namespace hg = hemlock::graphics;

#endif // __hemlock_graphics_font_instance_hpp
