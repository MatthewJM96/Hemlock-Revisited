#ifndef __hemlock_graphics_font_font_h
#define __hemlock_graphics_font_font_h

#include "graphics/font/instance.hpp"

namespace hemlock {
    namespace graphics {
        namespace font {
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
                 * @param file_ref The reference to the font's TTF file.
                 * @param start The first character to generate a glyph for.
                 * @param end The final character to generate a glyph for.
                 */
                void init(io::FileReference file_ref, char start, char end);
                /**
                 * @brief Initialises the font, after which it is ready to generate glyphs of specified sizes and styles.
                 *
                 * This version uses a default value for the first and final characters to generate glyphs for.
                 *     These characters are the printable characters in (non-extended) ASCII except for char 127.
                 *
                 * @param file_ref The reference to the font's TTF file.
                 */
                void init(io::FileReference file_ref) {
                    init(file_ref, FIRST_PRINTABLE_CHAR, LAST_PRINTABLE_CHAR);
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
                 * @param render_style The style with which the font should be rendered.
                 *
                 * @return True if a font instance was created, false indicates the font instance with the given
                 * properties already existed.
                 */
                bool generate(     FontSize size,
                                   FontSize padding,
                                  FontStyle style       = FontStyle::NORMAL,
                            FontRenderStyle render_style = FontRenderStyle::BLENDED );
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
                 * @param render_style The style with which the font should be rendered.
                 *
                 * @return True if a font instance was created, false indicates the font instance with the given
                 * properties already existed.
                 */
                bool generate(     FontSize size,
                                  FontStyle style       = FontStyle::NORMAL,
                            FontRenderStyle render_style = FontRenderStyle::BLENDED ) {
                    return generate(size, (size / 8) + 5, style, render_style);
                }
                /**
                 * @brief Generates a texture atlas of glyphs with the default render style, font style and font size.
                 *
                 * Note that all textures are of white glyphs, use shaders to tint them!
                 *
                 * @param style The style of the font itself.
                 * @param render_style The style with which the font should be rendered.
                 *
                 * @return True if a font instance was created, false indicates the font instance with the given
                 * properties already existed.
                 */
                bool generate(    FontStyle style       = FontStyle::NORMAL,
                            FontRenderStyle render_style = FontRenderStyle::BLENDED ) {
                    return generate(m_default_size, style, render_style);
                }

                /**
                 * @brief Returns the font instance corresponding to the given size, style and render
                 * style.
                 *
                 * @param size The font size.
                 * @param style The font style.
                 * @param render_style The font render style.
                 *
                 * @return The font instance corresponding to the given size, style and render style,
                 * or NIL_FONT_INSTANCE if no font instance exists with the given
                 */
                FontInstance get_instance(   FontSize size,
                                            FontStyle style       = FontStyle::NORMAL,
                                      FontRenderStyle render_style = FontRenderStyle::BLENDED );
                /**
                 * @brief Returns the font instance corresponding to the given size, style and render
                 * style.
                 *
                 * This version uses the default size of this font.
                 *
                 * @param style The font style.
                 * @param render_style The font render style.
                 *
                 * @return The font instance corresponding to the given size, style and render style,
                 * or NIL_FONT_INSTANCE if no font instance exists with the given
                 */
                FontInstance get_instance(    FontStyle style       = FontStyle::NORMAL,
                                        FontRenderStyle render_style = FontRenderStyle::BLENDED ) {
                    return get_instance(m_default_size, style, render_style);
                }

                /**
                 * @brief Disposes the font instance.
                 *
                 * @param font_instance The font instance to dispose.
                 *
                 * @return True if a font instance was found and disposed, false otherwise.
                 */
                bool dispose_instance(FontInstance font_instance);
                /**
                 * @brief Disposes the font instance.
                 *
                 * @param font_instance_hash The hash of the font instance to dispose.
                 *
                 * @return True if a font instance was found and disposed, false otherwise.
                 */
                bool dispose_instance(FontInstanceHash font_instance_hash);
                /**
                 * @brief Disposes the font instance corresponding to the given size, style and render
                 * style.
                 *
                 * @param size The font size.
                 * @param style The font style.
                 * @param render_style The font render style.
                 *
                 * @return True if a font instance was found and disposed, false otherwise.
                 */
                bool dispose_instance( FontSize size,
                                      FontStyle style       = FontStyle::NORMAL,
                                FontRenderStyle render_style = FontRenderStyle::BLENDED ) {
                    return dispose_instance(hash(size, style, render_style));
                }
                /**
                 * @brief Disposes the font instance corresponding to the given size, style and render
                 * style.
                 *
                 * This version uses the default size of this font.
                 *
                 * @param style The font style.
                 * @param render_style The font render style.
                 *
                 * @return True if a font instance was found and disposed, false otherwise.
                 */
                bool dispose_instance( FontStyle style       = FontStyle::NORMAL,
                                 FontRenderStyle render_style = FontRenderStyle::BLENDED ) {
                    return dispose_instance(m_default_size, style, render_style);
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

                io::FileReference   m_file_ref;
                char                m_start, m_end;
                FontSize            m_default_size;
                FontInstanceMap     m_font_instances;
            };

            class FontCache : public hio::Cache<Font, std::unordered_map<std::string, Font>> {};
        }
        namespace f = font;
    }
}
namespace hg = hemlock::graphics;

bool operator==(const hg::f::FontInstance& lhs, const hg::f::FontInstance& rhs);
bool operator!=(const hg::f::FontInstance& lhs, const hg::f::FontInstance& rhs);

// These are just a set of functions to let us use bit-masking for FontStyle.
//     That is to say, we can do things like:
//         FontStyle::BOLD | FontStyle::ITALIC
//     in order to specify we want a font instance that is bold AND italic!
hg::f::FontStyle  operator~  (hg::f::FontStyle rhs);
hg::f::FontStyle  operator|  (hg::f::FontStyle lhs,  hg::f::FontStyle rhs);
hg::f::FontStyle  operator&  (hg::f::FontStyle lhs,  hg::f::FontStyle rhs);
hg::f::FontStyle  operator^  (hg::f::FontStyle lhs,  hg::f::FontStyle rhs);
hg::f::FontStyle& operator|= (hg::f::FontStyle& lhs, hg::f::FontStyle rhs);
hg::f::FontStyle& operator&= (hg::f::FontStyle& lhs, hg::f::FontStyle rhs);
hg::f::FontStyle& operator^= (hg::f::FontStyle& lhs, hg::f::FontStyle rhs);

#endif // __hemlock_graphics_font_font_h
