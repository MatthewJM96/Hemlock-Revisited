#include "stdafx.h"

#include "graphics/font.h"

/**
 * @brief Determines the next power of 2 after the given value and returns it.
 *
 * @param The value to determine the following power of 2 for.
 *
 * @return The power of 2 determined.
 */
static ui32 next_power_2(ui32 value) {
    // This is a rather lovely bit manipulation function.
    // Essentially, all we're doing in this is up until the return
    // statement we take a value like 0110110000110101101 and change 
    // it to become 0111111111111111111 so that when we add 1 (i.e.
    // 0000000000000000001) it becomes 1000000000000000000 -> the
    // next power of 2!
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return ++value;
}

hg::FontInstanceHash hg::hash(FontSize size, FontStyle style, FontRenderStyle renderStyle) {
    FontInstanceHash hash = 0;

    // By ensuring FontInstanceHash has more bits that the sum of all three of the
    // provided values we can generate the hash simply by shifting the bits of style
    // and render style such that none of the three overlap.
    hash += static_cast<FontInstanceHash>(size);
    hash += static_cast<FontInstanceHash>(style)       << (sizeof(FontSize) * 8);
    hash += static_cast<FontInstanceHash>(renderStyle) << ((sizeof(FontSize) + sizeof(FontStyle)) * 8);

    return hash;
}

bool hg::FontInstance::save(const char* filepath, hio::image::Saver save) {
        // Prepare the pixel buffer.
        ui8* pixels = new ui8[textureSize.x * textureSize.y * 4];

        // Bind the texture, load it into our buffer, and then unbind it.
        glBindTexture(GL_TEXTURE_2D, texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        return save(filepath, static_cast<void*>(pixels), textureSize, hio::image::PixelFormat::RGBA_UI8);
}

hg::Font::Font() :
    m_filepath(nullptr),
    m_start(0), m_end(0),
    m_default_size(0)
{ /* Empty. */ }

void hg::Font::init(const char* filepath, char start, char end) {
    m_filepath = filepath;
    m_start    = start;
    m_end      = end;
}

void hg::Font::dispose() {
    for (auto& fontInstance : m_font_instances) {
        if (fontInstance.second.texture != 0) {
            glDeleteTextures(1, &fontInstance.second.texture);
        }
        if (fontInstance.second.glyphs != nullptr) {
            delete[] fontInstance.second.glyphs;
        }
    }

    FontInstanceMap().swap(m_font_instances);
}

bool hg::Font::generate( FontSize size,
                         FontSize padding,
                        FontStyle style       /*= FontStyle::NORMAL*/,
                  FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/ ) {
    // Make sure this is a new instance we are generating.
    if (get_instance(size, style, renderStyle) != NIL_FONT_INSTANCE) return false;

    // This is the font instance we will build up as we generate the texture atlas.
    FontInstance fontInstance{};
    // Create the glyphs array for this font instance.
    fontInstance.glyphs = new Glyph[m_end - m_start + 1];
    // Set this as the font instance's owner.
    fontInstance.owner = this;

    // Open the font and check we didn't fail.
    TTF_Font* font = TTF_OpenFont(m_filepath, size);
    if (font == nullptr) return false;

    // Set the font style.
    TTF_SetFontStyle(font, static_cast<int>(style));

    // Store the height of the tallest glyph for the given font size.
    fontInstance.height = TTF_FontHeight(font);

    // For each character, we are going to get the glyph metrics - that is the set of
    // properties that constitute begin and end positions of the glyph - and calculate
    // each glyph's size.
    {
        size_t i = 0;
        for (char c = m_start; c <= m_end; ++c) {
            fontInstance.glyphs[i].character = c;

            // Check that the glyph we are currently seeking actually gets provided
            // by the font in question.
            if (TTF_GlyphIsProvided(font, c) == 0) {
                fontInstance.glyphs[i].supported = false;

                ++i;
                continue;
            }

            // We will fill this with the glyph metrics, namely min & max values of
            // the X & Y coords of the font.
            //     metrics.x & metrics.y correspond to min & max X respectively.
            //     metrics.z & metrics.w correspond to min & max Y respectively.
            i32v4 metrics;

            TTF_GlyphMetrics(font, c, &metrics.x, &metrics.y,
                                    &metrics.z, &metrics.w, nullptr);

            // Calculate the glyph's sizes from the metric.
            fontInstance.glyphs[i].size.x = metrics.y - metrics.x;
            fontInstance.glyphs[i].size.y = metrics.w - metrics.z;

            // Given we got here, the glyph is supported.
            fontInstance.glyphs[i].supported = true;

            ++i;
        }
    }

    // Our texture atlas of all the glyphs in the font is going to have multiple rows.
    // We want to make this texture as small as possible in memory, so we now do some
    // preprocessing in order to find the number of rows that minimises the area of
    // the atlas (equivalent to the amount of data that will be used up by it).
    ui32 rowCount     = 1;
    ui32 bestWidth    = 0;
    ui32 bestHeight   = 0;
    ui32 bestArea     = std::numeric_limits<ui32>::max();
    ui32 bestRowCount = 0;
    Row* bestRows = nullptr;
    while (rowCount <= static_cast<ui32>(m_end - m_start)) {
        // WARNING: We may be packing too tightly here. The reported height of glyphs from TTF_GlyphMetrics
        //          is not always accurate to the rendered dimensions.

        // Generate rows for the current row count, getting the width and height of the rectangle
        // they form.
        ui32 currentWidth, currentHeight;
        Row* currentRows = generate_rows(fontInstance.glyphs, rowCount, padding, currentWidth, currentHeight);

        // There are benefits of making the texture larger to match power of 2 boundaries on
        // width and height.
        currentWidth  = next_power_2(currentWidth);
        currentHeight = next_power_2(currentHeight);

        // If the area of the rectangle drawn out by the rows generated is less than the previous
        // best area, then we have a new candidate!
        if (currentWidth * currentHeight < bestArea) {
            if (bestRows) delete[] bestRows;
            bestRows     = currentRows;
            bestWidth    = currentWidth;
            bestHeight   = currentHeight;
            bestRowCount = rowCount;
            bestArea     = bestWidth * bestHeight;
            ++rowCount;
        } else {
            // Area has increased, break out as going forwards it's likely area will only continue
            // to increase.
            delete[] currentRows;
            break;
        }
    }

    // Make sure we actually have rows to use.
    if (bestRows == nullptr) return false;

    // TODO(Matthew): Don't wanna be calling glGet every font gen... determine and store somewhere at initialisation.
    // Get maximum texture size allowed by implementation.
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    // If the best size texture we could get exceeds the largest texture area permitted by the GPU... fail.
    if (bestWidth * bestHeight > static_cast<ui32>(maxTextureSize * maxTextureSize)) {
        delete[] bestRows;
        return false;
    }

    // Set texture size in font instance.
    fontInstance.textureSize = ui32v2(bestWidth, bestHeight);

    // Generate & bind the texture we will put each glyph into.
    glGenTextures(1, &fontInstance.texture);
    glBindTexture(GL_TEXTURE_2D, fontInstance.texture);
    // Set the texture's size and pixel format.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bestWidth, bestHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set some needed parameters for the texture.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,     GL_REPEAT);

    // This represents the current V-coordinate we are into the texture.
    //    UV are the coordinates we use for textures (i.e. the X & Y coords of the pixels).
    ui32 currentV = padding;
    // Loop over all of the rows, for each going through and drawing each glyph,
    // adding it to our texture.
    for (size_t rowIndex = 0; rowIndex < bestRowCount; ++rowIndex) {
        // This represents the current U-coordinate we are into the texture.
        ui32 currentU = padding;
        for (size_t glyphIndex = 0; glyphIndex < bestRows[rowIndex].second.size(); ++glyphIndex) {
            ui32 charIndex = bestRows[rowIndex].second[glyphIndex];

            // If the glyph is unsupported, skip it!
            if (!fontInstance.glyphs[charIndex].supported) continue;

            // Determine which render style we are to use and draw the glyph.
            SDL_Surface* glyphSurface = nullptr;
            switch(renderStyle) {
                case FontRenderStyle::SOLID:
                    glyphSurface = TTF_RenderGlyph_Solid(font, static_cast<ui16>(m_start + charIndex), { 255, 255, 255, 255 });
                    break;
                case FontRenderStyle::BLENDED:
                    glyphSurface = TTF_RenderGlyph_Blended(font, static_cast<ui16>(m_start + charIndex), { 255, 255, 255, 255 });
                    break;
            }

            // Stitch the glyph we just generated into our texture.
            glTexSubImage2D(GL_TEXTURE_2D, 0, currentU, currentV, glyphSurface->w, glyphSurface->h, GL_BGRA, GL_UNSIGNED_BYTE, glyphSurface->pixels);

            // Update the size of the glyph with what we rendered - there can be variance between this and what we obtained
            // in the glyph metric stage!
            fontInstance.glyphs[charIndex].size.x = static_cast<f32>(glyphSurface->w);
            fontInstance.glyphs[charIndex].size.y = static_cast<f32>(glyphSurface->h);

            // Build the UV dimensions for the glyph.
            fontInstance.glyphs[charIndex].uvDimensions.x =        (static_cast<f32>(currentU) / static_cast<f32>(bestWidth));
            fontInstance.glyphs[charIndex].uvDimensions.y =        (static_cast<f32>(currentV) / static_cast<f32>(bestHeight));
            fontInstance.glyphs[charIndex].uvDimensions.z = (static_cast<f32>(glyphSurface->w) / static_cast<f32>(bestWidth));
            fontInstance.glyphs[charIndex].uvDimensions.w = (static_cast<f32>(glyphSurface->h) / static_cast<f32>(bestHeight));

            // Update currentU.
            currentU += glyphSurface->w + padding;

            // Free the glyph "surface".
            SDL_FreeSurface(glyphSurface);
            glyphSurface = nullptr;
        }
        // Update currentV.
        currentV += bestRows[rowIndex].first + padding;
    }

    // Clean up.
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] bestRows;

    // Note that this can fail for seemingly little reason.
    //     For example, if one tries to get glyph metrics for a character not provided.
    TTF_CloseFont(font);

    // Insert our font instance.
    m_font_instances.emplace(std::make_pair(hash(size, style, renderStyle), fontInstance));

    return true;
}

hg::FontInstance hg::Font::get_instance( FontSize size,
                                        FontStyle style       /*= FontStyle::NORMAL*/,
                                  FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/ ) {
    try {
        return m_font_instances.at(hash(size, style, renderStyle));
    } catch (std::out_of_range& e) {
        return NIL_FONT_INSTANCE;
    }
}

hg::Font::Row* hg::Font::generate_rows(Glyph* glyphs, ui32 rowCount, FontSize padding, ui32& width, ui32& height) {
    // Create some arrays for the rows, their widths and max height of a glyph within each of them.
    //    Max heights are stored inside Row - it is a pair of max height and a vector of glyph indices.
    Row*  rows          = new Row[rowCount]();
    ui32* currentWidths = new ui32[rowCount]();

    width  = padding;
    height = padding * rowCount + padding;
    // Initialise our arrays of widths and max heights.
    for (size_t i = 0; i < rowCount; ++i) {
        currentWidths[i] = padding;
        rows[i].first    = 0;
    }

    // For each character, we now determine which row to put it in, updating the width and
    // height variables as we go.
    for (ui32 i = 0; i < static_cast<ui32>(m_end - m_start); ++i) {
        // Skip unsupported glyphs.
        if (!glyphs[i].supported) continue;

        // Determine which row currently has the least width: this is the row we will add
        // the currently considered glyph to.
        size_t bestRow = 0;
        for (size_t j = 1; j < rowCount; ++j) {
            // If row with index j is not as wide as the current least-wide row then it becomes
            // the new least-wide row!
            if (currentWidths[bestRow] > currentWidths[j]) bestRow = j;
        }

        // Update the width of the row we have chosen to add the glyph to.
        currentWidths[bestRow] += glyphs[i].size.x + padding;

        // Update the overall width of the rectangle the rows form,
        // if our newly enlarged row exceeds it.
        if (width < currentWidths[bestRow]) width = currentWidths[bestRow];

        // Update the max height of our row if the new glyph exceeds it, and update
        // the height of the rectange the rows form.
        if (rows[bestRow].first < glyphs[i].size.y) {
            height -= rows[bestRow].first;
            height += glyphs[i].size.y;
            rows[bestRow].first = glyphs[i].size.y;
        }

        rows[bestRow].second.push_back(i);
    }

    // Clear up memory.
    delete[] currentWidths;

    // Return the rows we've built up!
    return rows;
}

void hg::FontCache::dispose() {
    // Dispose the cached fonts.
    for (auto& font : m_fonts) {
        font.second.dispose();
    }

    // Empty our map of fonts.
    Fonts().swap(m_fonts);
}

bool hg::FontCache::register_font(const char* name, const char* filepath, char start, char end) {
    auto it = m_fonts.find(name);
    if (it != m_fonts.end()) {
        return false;
    }

    m_fonts[name] = Font{};
    m_fonts[name].init(filepath, start, end);

    return true;
}
bool hg::FontCache::register_font(const char* name, const char* filepath) {
    // Try to emplace a new Font object with the given name.
    auto [_, added] = m_fonts.try_emplace(name, Font());
    // If we added it, then initialise the Font object.
    if (added) {
        m_fonts.at(name).init(filepath);
        return true;
    }
    return false;
}

hg::FontInstance hg::FontCache::fetch( const char* name,
                                          FontSize size,
                                         FontStyle style     /*= FontStyle::NORMAL*/,
                                   FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    // Make sure a font exists with the given name.
    auto font = m_fonts.find(name);
    if (font == m_fonts.end()) return NIL_FONT_INSTANCE;

    // Generate the specified font instance if it doesn't exist.
    font->second.generate(size, style, renderStyle);

    // Return the font instance.
    return font->second.get_instance(size, style, renderStyle);
}

hg::FontInstance hg::FontCache::fetch( const char* name,
                                       const char* filepath,
                                          FontSize size,
                                         FontStyle style     /*= FontStyle::NORMAL*/,
                                   FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    register_font(name, filepath);

    return fetch(name, size, style, renderStyle);
}

hg::FontInstance hg::FontCache::fetch( const char* name,
                                         FontStyle style     /*= FontStyle::NORMAL*/,
                                   FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    // Make sure a font exists with the given name.
    auto font = m_fonts.find(name);
    if (font == m_fonts.end()) return NIL_FONT_INSTANCE;

    // Generate the specified font instance if it doesn't exist.
    font->second.generate(style, renderStyle);

    // Return the font instance.
    return font->second.get_instance(style, renderStyle);
}

hg::FontInstance hg::FontCache::fetch( const char* name,
                                       const char* filepath,
                                         FontStyle style       /*= FontStyle::NORMAL*/,
                                   FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    register_font(name, filepath);

    return fetch(name, style, renderStyle);
}

bool operator==(const hg::FontInstance& lhs, const hg::FontInstance& rhs) {
    return (lhs.texture == rhs.texture &&
            lhs.height  == rhs.height  &&
            lhs.glyphs  == rhs.glyphs);
}
bool operator!=(const hg::FontInstance& lhs, const hg::FontInstance& rhs) {
    return !(lhs == rhs);
}

// These are just a set of functions to let us use bit-masking for FontStyle.
//     That is to say, we can do things like:
//         FontStyle::BOLD | FontStyle::ITALIC
//     in order to specify we want a font instance that is bold AND italic!
hg::FontStyle operator~ (hg::FontStyle rhs) {
    return static_cast<hg::FontStyle>(
        ~static_cast<std::underlying_type<hg::FontStyle>::type>(rhs)
    );
}
hg::FontStyle operator| (hg::FontStyle lhs, hg::FontStyle rhs) {
    return static_cast<hg::FontStyle>(
        static_cast<std::underlying_type<hg::FontStyle>::type>(lhs) |
        static_cast<std::underlying_type<hg::FontStyle>::type>(rhs)
    );
}
hg::FontStyle operator& (hg::FontStyle lhs, hg::FontStyle rhs) {
    return static_cast<hg::FontStyle>(
        static_cast<std::underlying_type<hg::FontStyle>::type>(lhs) &
        static_cast<std::underlying_type<hg::FontStyle>::type>(rhs)
    );
}
hg::FontStyle operator^ (hg::FontStyle lhs, hg::FontStyle rhs) {
    return static_cast<hg::FontStyle>(
        static_cast<std::underlying_type<hg::FontStyle>::type>(lhs) ^
        static_cast<std::underlying_type<hg::FontStyle>::type>(rhs)
    );
}
hg::FontStyle& operator|= (hg::FontStyle& lhs, hg::FontStyle rhs) {
    lhs = static_cast<hg::FontStyle>(
        static_cast<std::underlying_type<hg::FontStyle>::type>(lhs) |
        static_cast<std::underlying_type<hg::FontStyle>::type>(rhs)
    );

    return lhs;
}
hg::FontStyle& operator&= (hg::FontStyle& lhs, hg::FontStyle rhs) {
    lhs = static_cast<hg::FontStyle>(
        static_cast<std::underlying_type<hg::FontStyle>::type>(lhs) &
        static_cast<std::underlying_type<hg::FontStyle>::type>(rhs)
    );

    return lhs;
}
hg::FontStyle& operator^= (hg::FontStyle& lhs, hg::FontStyle rhs) {
    lhs = static_cast<hg::FontStyle>(
        static_cast<std::underlying_type<hg::FontStyle>::type>(lhs) ^
        static_cast<std::underlying_type<hg::FontStyle>::type>(rhs)
    );

    return lhs;
}
