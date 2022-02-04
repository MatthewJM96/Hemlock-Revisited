#ifndef __hemlock_graphics_font_h
#define __hemlock_graphics_font_h

#include "io/image.h"

namespace hemlock {
    namespace graphics {
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
        using StringComponents = std::vector<std::pair<const char*, StringDrawProperties>>;

        // TODO(Matthew): Implement font instance disposal.
        //                    We wanna release memory we are using as soon as we don't need it!
        // TODO(Matthew): Implement packing of font textures?
        //                    At one level this can be done inside Font by generating multiple font instances simultaneously in a pack,
        //                    at a second, we could let the cacher request all the font instances it has cached be packed together?
        //                        Packing the fonts will reduce the number of drawcalls needed in text using multiple font instances (e.g. bold and non-bold text)
        /**
         * @brief Handles a single font (defined by a single TTF file), for which textures
         *        may be generated for variations of font size and style.
         */
        class Font {
            using FontInstanceMap = std::unordered_map<FontInstanceHash, FontInstance>;
        public:
            using Row = std::pair<ui32, std::vector<ui32>>;

            Font();
            ~Font() { /* Empty. */ }

            /**
             * @brief Initialises the font, after which it is ready to generate glyphs of specified sizes and styles.
             *
             * @param filepath The path to the font's TTF file.
             * @param start The first character to generate a glyph for.
             * @param end The final character to generate a glyph for.
             */
            void init(std::string filepath, char start, char end);
            /**
             * @brief Initialises the font, after which it is ready to generate glyphs of specified sizes and styles.
             *
             * This version uses a default value for the first and final characters to generate glyphs for.
             *     These characters are the printable characters in (non-extended) ASCII except for char 127.
             *
             * @param filepath The path to the font's TTF file.
             */
            void init(std::string filepath) {
                init(filepath, FIRST_PRINTABLE_CHAR, LAST_PRINTABLE_CHAR);
            }
            /**
             * @brief Disposes of the font and all variations for which textures were generated.
             */
            void dispose();

            char get_start() { return m_start; }
            char get_end()   { return m_end;   }

            FontSize get_default_size()              { return m_default_size; }
            void     set_default_size(FontSize size) { m_default_size = size; }

            /**
             * @brief Generates a texture atlas of glyphs with the given render style, font style and font size.
             *
             * Note that all textures are of white glyphs, use shaders to tint them!
             *
             * @param size The size of the glyphs to be drawn.
             * @param size The padding to use to space out the glyphs to be drawn.
             * @param style The style of the font itself.
             * @param renderStyle The style with which the font should be rendered.
             *
             * @return True if a font instance was created, false indicates the font instance with the given
             * properties already existed.
             */
            bool generate(       FontSize size,
                                 FontSize padding,
                                FontStyle style       = FontStyle::NORMAL,
                          FontRenderStyle renderStyle = FontRenderStyle::BLENDED );
            /**
             * @brief Generates a texture atlas of glyphs with the given render style, font style and font size.
             *
             * Note that all textures are of white glyphs, use shaders to tint them!
             *
             * This variation makes a reasonable guess at a padding to use between glyphs;
             * in most cases it should not be necessary to specify a different padding.
             *
             * @param size The size of the glyphs to be drawn.
             * @param style The style of the font itself.
             * @param renderStyle The style with which the font should be rendered.
             *
             * @return True if a font instance was created, false indicates the font instance with the given
             * properties already existed.
             */
            bool generate(       FontSize size,
                                FontStyle style       = FontStyle::NORMAL,
                          FontRenderStyle renderStyle = FontRenderStyle::BLENDED ) {
                return generate(size, (size / 8) + 5, style, renderStyle);
            }
            /**
             * @brief Generates a texture atlas of glyphs with the default render style, font style and font size.
             *
             * Note that all textures are of white glyphs, use shaders to tint them!
             *
             * @param style The style of the font itself.
             * @param renderStyle The style with which the font should be rendered.
             *
             * @return True if a font instance was created, false indicates the font instance with the given
             * properties already existed.
             */
            bool generate(      FontStyle style       = FontStyle::NORMAL,
                          FontRenderStyle renderStyle = FontRenderStyle::BLENDED ) {
                return generate(m_default_size, style, renderStyle);
            }

            /**
             * @brief Returns the font instance corresponding to the given size, style and render
             * style.
             *
             * @param size The font size.
             * @param style The font style.
             * @param renderStyle The font render style.
             *
             * @return The font instance corresponding to the given size, style and render style,
             * or NIL_FONT_INSTANCE if no font instance exists with the given
             */
            FontInstance get_instance(     FontSize size,
                                          FontStyle style       = FontStyle::NORMAL,
                                    FontRenderStyle renderStyle = FontRenderStyle::BLENDED );
            /**
             * @brief Returns the font instance corresponding to the given size, style and render
             * style.
             *
             * This version uses the default size of this font.
             *
             * @param style The font style.
             * @param renderStyle The font render style.
             *
             * @return The font instance corresponding to the given size, style and render style,
             * or NIL_FONT_INSTANCE if no font instance exists with the given
             */
            FontInstance get_instance(    FontStyle style       = FontStyle::NORMAL,
                                    FontRenderStyle renderStyle = FontRenderStyle::BLENDED ) {
                return get_instance(m_default_size, style, renderStyle);
            }
        protected:
            /**
             * @brief Generates as many rows of glyphs as requested, ensuring 
             * each row is as similarly wide as every other row.
             *
             * @param glyphs The size of each glyph to be fit into the rows.
             * @param rowCount The number of rows to split the glyphs into.
             * @param padding The padding to be placed between each glyph.
             * @param width This is set to the width of the longest row generated.
             * @param height This is set to the sum of the max height of each glyph in each row.
             */
            Row* generate_rows(Glyph* glyphs, ui32 rowCount, FontSize padding, ui32& width, ui32& height);

            std::string     m_filepath;
            char            m_start, m_end;
            FontSize        m_default_size;
            FontInstanceMap m_font_instances;
        };

        class FontCache : public Cache<Font, std::unordered_map<std::string, Font>> {};
    }
}
namespace hg = hemlock::graphics;

bool operator==(const hg::FontInstance& lhs, const hg::FontInstance& rhs);
bool operator!=(const hg::FontInstance& lhs, const hg::FontInstance& rhs);

// These are just a set of functions to let us use bit-masking for FontStyle.
//     That is to say, we can do things like:
//         FontStyle::BOLD | FontStyle::ITALIC
//     in order to specify we want a font instance that is bold AND italic!
hg::FontStyle  operator~  (hg::FontStyle rhs);
hg::FontStyle  operator|  (hg::FontStyle lhs,  hg::FontStyle rhs);
hg::FontStyle  operator&  (hg::FontStyle lhs,  hg::FontStyle rhs);
hg::FontStyle  operator^  (hg::FontStyle lhs,  hg::FontStyle rhs);
hg::FontStyle& operator|= (hg::FontStyle& lhs, hg::FontStyle rhs);
hg::FontStyle& operator&= (hg::FontStyle& lhs, hg::FontStyle rhs);
hg::FontStyle& operator^= (hg::FontStyle& lhs, hg::FontStyle rhs);

#endif // __hemlock_graphics_font_h
