#include "stdafx.h"

#include "graphics/pixel.h"

#include "graphics/font/font.h"

/**
 * @brief Enumeration of styles of fonts.
 */
H_DEF_ENUM_WITH_SERIALISATION(hemlock::graphics::font, FontStyle)

/**
 * @brief Enumeration of styles of font rendering.
 */
H_DEF_ENUM_WITH_SERIALISATION(hemlock::graphics::font, FontRenderStyle)

hg::f::FontInstanceHash
hg::f::hash(FontSize size, FontStyle style, FontRenderStyle render_style) {
    FontInstanceHash hash = 0;

    // By ensuring FontInstanceHash has more bits that the sum of all three of the
    // provided values we can generate the hash simply by shifting the bits of style
    // and render style such that none of the three overlap.
    hash += static_cast<FontInstanceHash>(size);
    hash += static_cast<FontInstanceHash>(style) << (sizeof(FontSize) * 8);
    hash += static_cast<FontInstanceHash>(render_style)
            << ((sizeof(FontSize) + sizeof(FontStyle)) * 8);

    return hash;
}

bool hg::f::FontInstance::save(std::string filepath, hio::image::Saver save) {
    // Prepare the pixel buffer.
    ui8* pixels = new ui8[texture_size.x * texture_size.y * 4];

    // Bind the texture, load it into our buffer, and then unbind it.
#if !defined(HEMLOCK_OS_MAC)
    glGetTextureImage(
        texture,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        texture_size.x * texture_size.y * 4,
        pixels
    );
#else   // !defined(HEMLOCK_OS_MAC)
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0)
#endif  // !defined(HEMLOCK_OS_MAC)

    return save(filepath, pixels, texture_size, hio::image::PixelFormat::RGBA_UI8);
}

hg::f::Font::Font() :
    m_filepath(""), m_start(0), m_end(0), m_default_size(0) { /* Empty. */
}

void hg::f::Font::init(std::string filepath, char start, char end) {
    m_filepath = filepath;
    m_start    = start;
    m_end      = end;
}

void hg::f::Font::dispose() {
    for (auto& instance : m_font_instances) {
        if (instance.second.texture != 0) {
            glDeleteTextures(1, &instance.second.texture);
        }
        if (instance.second.glyphs != nullptr) {
            delete[] instance.second.glyphs;
        }
    }

    FontInstanceMap().swap(m_font_instances);
}

bool hg::f::Font::generate(
    FontSize        size,
    FontSize        padding,
    FontStyle       style /*= FontStyle::NORMAL*/,
    FontRenderStyle render_style /*= FontRenderStyle::BLENDED*/
) {
    // Make sure this is a new instance we are generating.
    if (get_instance(size, style, render_style) != NIL_FONT_INSTANCE) return false;

    // This is the font instance we will build up as we generate the texture atlas.
    FontInstance font_instance{};
    // Create the glyphs array for this font instance.
    font_instance.glyphs = new Glyph[m_end - m_start + 1];
    // Set this as the font instance's owner.
    font_instance.owner = this;

    // Open the font and check we didn't fail.
    TTF_Font* font = TTF_OpenFont(m_filepath.data(), size);
    if (font == nullptr) return false;

    // Set the font style.
    TTF_SetFontStyle(font, static_cast<int>(style));

    // Store the height of the tallest glyph for the given font size.
    font_instance.height = TTF_FontHeight(font);

    // For each character, we are going to get the glyph metrics - that is the set of
    // properties that constitute begin and end positions of the glyph - and calculate
    // each glyph's size.
    {
        size_t i = 0;
        for (char c = m_start; c <= m_end; ++c) {
            font_instance.glyphs[i].character = c;

            // Check that the glyph we are currently seeking actually gets provided
            // by the font in question.
            if (TTF_GlyphIsProvided(font, c) == 0) {
                font_instance.glyphs[i].supported = false;

                ++i;
                continue;
            }

            // We will fill this with the glyph metrics, namely min & max values of
            // the X & Y coords of the font.
            //     metrics.x & metrics.y correspond to min & max X respectively.
            //     metrics.z & metrics.w correspond to min & max Y respectively.
            i32v4 metrics;

            TTF_GlyphMetrics(
                font, c, &metrics.x, &metrics.y, &metrics.z, &metrics.w, nullptr
            );

            // Calculate the glyph's sizes from the metric.
            font_instance.glyphs[i].size.x = static_cast<f32>(metrics.y - metrics.x);
            font_instance.glyphs[i].size.y = static_cast<f32>(metrics.w - metrics.z);

            // Given we got here, the glyph is supported.
            font_instance.glyphs[i].supported = true;

            ++i;
        }
    }

    // Our texture atlas of all the glyphs in the font is going to have multiple rows.
    // We want to make this texture as small as possible in memory, so we now do some
    // preprocessing in order to find the number of rows that minimises the area of
    // the atlas (equivalent to the amount of data that will be used up by it).
    ui32 row_count      = 1;
    ui32 best_width     = 0;
    ui32 best_height    = 0;
    ui32 best_area      = std::numeric_limits<ui32>::max();
    ui32 best_row_count = 0;
    Row* best_rows      = nullptr;
    while (row_count <= static_cast<ui32>(m_end - m_start)) {
        // WARNING: We may be packing too tightly here. The reported height of glyphs
        // from TTF_GlyphMetrics
        //          is not always accurate to the rendered dimensions.

        // Generate rows for the current row count, getting the width and height of
        // the rectangle they form.
        ui32 current_width, current_height;
        Row* current_rows = generate_rows(
            font_instance.glyphs, row_count, padding, current_width, current_height
        );

        // There are benefits of making the texture larger to match power of 2
        // boundaries on width and height.
        current_width  = hmaths::next_power_2(current_width);
        current_height = hmaths::next_power_2(current_height);

        // If the area of the rectangle drawn out by the rows generated is less than
        // the previous best area, then we have a new candidate!
        if (current_width * current_height < best_area) {
            if (best_rows) delete[] best_rows;
            best_rows      = current_rows;
            best_width     = current_width;
            best_height    = current_height;
            best_row_count = row_count;
            best_area      = best_width * best_height;
            ++row_count;
        } else {
            // Area has increased, break out as going forwards it's likely area will
            // only continue to increase.
            delete[] current_rows;
            break;
        }
    }

    // Make sure we actually have rows to use.
    if (best_rows == nullptr) return false;

    // TODO(Matthew): Don't wanna be calling glGet every font gen... determine and
    // store somewhere at initialisation. Get maximum texture size allowed by
    // implementation.
    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

    // If the best size texture we could get exceeds the largest texture area
    // permitted by the GPU... fail.
    if (best_width * best_height
        > static_cast<ui32>(max_texture_size * max_texture_size))
    {
        delete[] best_rows;
        return false;
    }

    // Set texture size in font instance.
    font_instance.texture_size = ui32v2(best_width, best_height);

#if !defined(HEMLOCK_OS_MAC)
    // Create the texture we will put each glyph into.
    glCreateTextures(GL_TEXTURE_2D, 1, &font_instance.texture);
    // Set the texture's size and pixel format.
    glTextureStorage2D(font_instance.texture, 1, GL_RGBA8, best_width, best_height);

    // Note that by default MAG_FILTER, WRAP_* are as we are setting them,
    // the crucial parameter to set is MIN_FILTER that defaults to a
    // mip mapping setting that won't work for these textures, causing
    // OpenGL to treat pixels all as RGBA{0,0,0,255}.
    glTextureParameteri(font_instance.texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(font_instance.texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(font_instance.texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(font_instance.texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(font_instance.texture, GL_TEXTURE_WRAP_R, GL_REPEAT);
#else   // !defined(HEMLOCK_OS_MAC)
    // Create the texture we will put each glyph into.
    glGenTextures(1, &font_instance.texture);
    glBindTexture(GL_TEXTURE_2D, font_instance.texture);
    // Set the texture's size and pixel format.
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, best_width, best_height);

    // Note that by default MAG_FILTER, WRAP_* are as we are setting them,
    // the crucial parameter to set is MIN_FILTER that defaults to a
    // mip mapping setting that won't work for these textures, causing
    // OpenGL to treat pixels all as RGBA{0,0,0,255}.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
#endif  // !defined(HEMLOCK_OS_MAC)

    // This represents the current V-coordinate we are into the texture.
    //    UV are the coordinates we use for textures (i.e. the X & Y coords of the
    //    pixels).
    ui32 current_v = padding;
    // Loop over all of the rows, for each going through and drawing each glyph,
    // adding it to our texture.
    for (size_t row_index = 0; row_index < best_row_count; ++row_index) {
        // This represents the current U-coordinate we are into the texture.
        ui32 current_u = padding;
        for (size_t glyph_index = 0; glyph_index < best_rows[row_index].second.size();
             ++glyph_index)
        {
            ui32 char_index = best_rows[row_index].second[glyph_index];

            // If the glyph is unsupported, skip it!
            if (!font_instance.glyphs[char_index].supported) continue;

            // Determine which render style we are to use and draw the glyph.
            SDL_Surface* glyph_surface = nullptr;
            switch (render_style) {
                case FontRenderStyle::SOLID:
                    glyph_surface = TTF_RenderGlyph_Solid(
                        font,
                        static_cast<ui16>(m_start + char_index),
                        { 255, 255, 255, 255 }
                    );
                    break;
                case FontRenderStyle::BLENDED:
                    glyph_surface = TTF_RenderGlyph_Blended(
                        font,
                        static_cast<ui16>(m_start + char_index),
                        { 255, 255, 255, 255 }
                    );
                    break;
                default:
                    debug_printf("Trying to render a font with invalid render style.");
                    return false;
                    break;
            }

            /*
             * SDL_ttf produces SDL_Surfaces with indexing into a colour palette for
             * solid case, and.ARGB8888 for blended case now. Let's handle both
             * indexed colour palette and general 4-byte cases and hope that covers
             * us.
             */
            ui8* actual_pixels = nullptr;
            switch (glyph_surface->format->BytesPerPixel) {
                case 1:
                    p::convert_sdl_indexed_to_rgba_8888(glyph_surface, actual_pixels);
                    break;
                case 4:
                    p::convert_sdl_xxxx_8888_to_rgba_8888(glyph_surface, actual_pixels);
                    break;
            }
            if (actual_pixels == nullptr) return false;

#if !defined(HEMLOCK_OS_MAC)
            // Stitch the glyph we just generated into our texture.
            glTextureSubImage2D(
                font_instance.texture,
                0,
                current_u,
                current_v,
                glyph_surface->w,
                glyph_surface->h,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                actual_pixels
            );
#else   // !defined(HEMLOCK_OS_MAC)
        // Stitch the glyph we just generated into our texture.
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                current_u,
                current_v,
                glyph_surface->w,
                glyph_surface->h,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                actual_pixels
            );
#endif  // !defined(HEMLOCK_OS_MAC)

            // Update the size of the glyph with what we rendered - there can be
            // variance between this and what we obtained in the glyph metric stage!
            font_instance.glyphs[char_index].size.x
                = static_cast<f32>(glyph_surface->w);
            font_instance.glyphs[char_index].size.y
                = static_cast<f32>(glyph_surface->h);

            // Build the UV dimensions for the glyph.
            font_instance.glyphs[char_index].uv_rect.x
                = (static_cast<f32>(current_u) / static_cast<f32>(best_width));
            font_instance.glyphs[char_index].uv_rect.y
                = (static_cast<f32>(current_v) / static_cast<f32>(best_height));
            font_instance.glyphs[char_index].uv_rect.z
                = (static_cast<f32>(glyph_surface->w) / static_cast<f32>(best_width));
            font_instance.glyphs[char_index].uv_rect.w
                = (static_cast<f32>(glyph_surface->h) / static_cast<f32>(best_height));

            // Update current_u.
            current_u += glyph_surface->w + padding;

            // Free the glyph "surface".
            SDL_FreeSurface(glyph_surface);
            glyph_surface = nullptr;
            delete[] actual_pixels;
        }
        // Update current_v.
        current_v += best_rows[row_index].first + padding;
    }

#if defined(HEMLOCK_OS_MAC)
    glBindTexture(GL_TEXTURE_2D, 0);
#endif  // !defined(HEMLOCK_OS_MAC)

    // Clean up.
    delete[] best_rows;

    // Note that this can fail for seemingly little reason.
    //     For example, if one tries to get glyph metrics for a character not
    //     provided.
    TTF_CloseFont(font);

    // Insert our font instance.
    m_font_instances.emplace(
        std::make_pair(hash(size, style, render_style), font_instance)
    );

    return true;
}

hg::f::FontInstance hg::f::Font::get_instance(
    FontSize        size,
    FontStyle       style /*= FontStyle::NORMAL*/,
    FontRenderStyle render_style /*= FontRenderStyle::BLENDED*/
) {
    try {
        return m_font_instances.at(hash(size, style, render_style));
    } catch (std::out_of_range&) {
        return NIL_FONT_INSTANCE;
    }
}

bool hg::f::Font::dispose_instance(FontInstance font_instance) {
    auto erased = std::erase_if(m_font_instances, [font_instance](auto& lhs) {
        return lhs.second == font_instance;
    });
    if (erased == 0) return false;

    glDeleteTextures(1, &font_instance.texture);
    delete[] font_instance.glyphs;

    return true;
}

bool hg::f::Font::dispose_instance(FontInstanceHash font_instance_hash) {
    return static_cast<bool>(m_font_instances.erase(font_instance_hash));
}

hg::f::Font::Row* hg::f::Font::generate_rows(
    Glyph* glyphs, ui32 row_count, FontSize padding, ui32& width, ui32& height
) {
    // Create some arrays for the rows, their widths and max height of a glyph within
    // each of them.
    //    Max heights are stored inside Row - it is a pair of max height and a vector
    //    of glyph indices.
    Row*  rows           = new Row[row_count]();
    ui32* current_widths = new ui32[row_count]();

    width  = padding;
    height = padding * row_count + padding;
    // Initialise our arrays of widths and max heights.
    for (size_t i = 0; i < row_count; ++i) {
        current_widths[i] = padding;
        rows[i].first     = 0;
    }

    // For each character, we now determine which row to put it in, updating the width
    // and height variables as we go.
    for (ui32 i = 0; i < static_cast<ui32>(m_end - m_start); ++i) {
        // Skip unsupported glyphs.
        if (!glyphs[i].supported) continue;

        // Determine which row currently has the least width: this is the row we will
        // add the currently considered glyph to.
        size_t bestRow = 0;
        for (size_t j = 1; j < row_count; ++j) {
            // If row with index j is not as wide as the current least-wide row then
            // it becomes the new least-wide row!
            if (current_widths[bestRow] > current_widths[j]) bestRow = j;
        }

        // Update the width of the row we have chosen to add the glyph to.
        current_widths[bestRow] += static_cast<ui32>(glyphs[i].size.x) + padding;

        // Update the overall width of the rectangle the rows form,
        // if our newly enlarged row exceeds it.
        if (width < current_widths[bestRow]) width = current_widths[bestRow];

        // Update the max height of our row if the new glyph exceeds it, and update
        // the height of the rectange the rows form.
        if (rows[bestRow].first < glyphs[i].size.y) {
            height              -= rows[bestRow].first;
            height              += static_cast<ui32>(glyphs[i].size.y);
            rows[bestRow].first = static_cast<ui32>(glyphs[i].size.y);
        }

        rows[bestRow].second.push_back(i);
    }

    // Clear up memory.
    delete[] current_widths;

    // Return the rows we've built up!
    return rows;
}

bool operator==(const hg::f::FontInstance& lhs, const hg::f::FontInstance& rhs) {
    return (
        lhs.texture == rhs.texture && lhs.height == rhs.height
        && lhs.glyphs == rhs.glyphs
    );
}

bool operator!=(const hg::f::FontInstance& lhs, const hg::f::FontInstance& rhs) {
    return !(lhs == rhs);
}

// These are just a set of functions to let us use bit-masking for FontStyle.
//     That is to say, we can do things like:
//         FontStyle::BOLD | FontStyle::ITALIC
//     in order to specify we want a font instance that is bold AND italic!
hg::f::FontStyle operator~(hg::f::FontStyle rhs) {
    return static_cast<hg::f::FontStyle>(
        ~static_cast<std::underlying_type<hg::f::FontStyle>::type>(rhs)
    );
}

hg::f::FontStyle operator|(hg::f::FontStyle lhs, hg::f::FontStyle rhs) {
    return static_cast<hg::f::FontStyle>(
        static_cast<std::underlying_type<hg::f::FontStyle>::type>(lhs)
        | static_cast<std::underlying_type<hg::f::FontStyle>::type>(rhs)
    );
}

hg::f::FontStyle operator&(hg::f::FontStyle lhs, hg::f::FontStyle rhs) {
    return static_cast<hg::f::FontStyle>(
        static_cast<std::underlying_type<hg::f::FontStyle>::type>(lhs)
        & static_cast<std::underlying_type<hg::f::FontStyle>::type>(rhs)
    );
}

hg::f::FontStyle operator^(hg::f::FontStyle lhs, hg::f::FontStyle rhs) {
    return static_cast<hg::f::FontStyle>(
        static_cast<std::underlying_type<hg::f::FontStyle>::type>(lhs)
        ^ static_cast<std::underlying_type<hg::f::FontStyle>::type>(rhs)
    );
}

hg::f::FontStyle& operator|=(hg::f::FontStyle& lhs, hg::f::FontStyle rhs) {
    lhs = static_cast<hg::f::FontStyle>(
        static_cast<std::underlying_type<hg::f::FontStyle>::type>(lhs)
        | static_cast<std::underlying_type<hg::f::FontStyle>::type>(rhs)
    );

    return lhs;
}

hg::f::FontStyle& operator&=(hg::f::FontStyle& lhs, hg::f::FontStyle rhs) {
    lhs = static_cast<hg::f::FontStyle>(
        static_cast<std::underlying_type<hg::f::FontStyle>::type>(lhs)
        & static_cast<std::underlying_type<hg::f::FontStyle>::type>(rhs)
    );

    return lhs;
}

hg::f::FontStyle& operator^=(hg::f::FontStyle& lhs, hg::f::FontStyle rhs) {
    lhs = static_cast<hg::f::FontStyle>(
        static_cast<std::underlying_type<hg::f::FontStyle>::type>(lhs)
        ^ static_cast<std::underlying_type<hg::f::FontStyle>::type>(rhs)
    );

    return lhs;
}
