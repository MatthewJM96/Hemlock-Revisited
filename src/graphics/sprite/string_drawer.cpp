#include "stdafx.h"

#include "graphics/clipping.hpp"
#include "graphics/font/font.h"
#include "graphics/sprite/batcher.h"

#include "graphics/sprite/string_drawer.h"

using namespace hg::f;
using namespace hg::s;

H_DEF_ENUM_WITH_SERIALISATION(hemlock::graphics::font, StringSizingKind)

/******************************************************\
 * No Wrap Draw                                       *
\******************************************************/

void hg::add_string_no_wrap(SpriteBatcher* batcher, DrawableStringComponent component, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
    add_string_no_wrap(batcher, &component, 1, target_rect, clip_rect, align, depth);
}

void hg::add_string_no_wrap(SpriteBatcher* batcher, DrawableStringComponents components, ui32 num_components, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
    // We will populate these data points for drawing later.
    DrawableLines lines;
    f32 total_height = 0.0f;

    // Place the first line.
    lines.emplace_back(DrawableLine{0.0f, 0.0f, std::vector<DrawableGlyph>()});

    // TODO(Matthew): Can we make guesses as to the amount of drawables to reserve for a line? For amount of lines?

    for (ui32 component_idx = 0; component_idx < num_components; ++component_idx) {
        auto& component = components[component_idx];

        // Simplify property names.
        const char*  str    = component.str;
        FontInstance font   = component.props.font_instance;
        StringSizing sizing = component.props.sizing;
        colour4      tint   = component.props.tint;

        char start = font.owner->get_start();
        char end   = font.owner->get_end();

        // Process sizing into a simple scale factor.
        f32v2 scaling;
        f32   height;
        if (sizing.kind == StringSizingKind::SCALED) {
            scaling    = sizing.scaling;
            height = static_cast<f32>(font.height) * scaling.y;
        } else {
            scaling.x  = sizing.scale_x;
            scaling.y  = sizing.target_height / static_cast<f32>(font.height);
            height = sizing.target_height;
        }

        // Gets set to true if we go out of the height of the rect.
        bool vertical_overflow = false;
        // Iterate over this component's string.
        for (size_t i = 0; str[i] != '\0'; ++i) {
            char   character       = str[i];
            size_t character_index = static_cast<size_t>(character) - static_cast<size_t>(start);

            // If character is a new line character, add a new line and go to next character.
            if (character == '\n') {
                total_height += lines.back().height;
                lines.emplace_back(DrawableLine{ 0.0f, height, std::vector<DrawableGlyph>() });

                continue;
            }

            // TODO(Matthew): default alternative to indicate font issue?
            // If character is unsupported, skip.
            if (character < start || character > end ||
                    !font.glyphs[character_index].supported) continue;

            // If the line's height is less than the height of this font instance, and we're about to add a
            // glyph from this font instance, then the line's height needs setting to the font instances's.
            if (lines.back().height < height) {
                // If we have overflowed the rectangle with the new line height, break out instead.
                if (total_height + height > target_rect.w) {
                    vertical_overflow = true;
                    break;
                }
                lines.back().height = height;
            }

            // Determine character width after scaling.
            f32 character_width = font.glyphs[character_index].size.x * scaling.x;

            // Add character to line for drawing.
            lines.back().drawables.emplace_back(DrawableGlyph{ &font.glyphs[character_index], lines.back().length, scaling, tint, font.texture });
            lines.back().length += character_width;
        }

        // If we have overflown vertically, break out of outer loop.
        if (vertical_overflow) break;
    }
    // Update the total height for last line.
    total_height += lines.back().height;

    f32 current_y = 0.0f;
    for (auto& line : lines) {
        f32v2 offsets = calculate_align_offset(align, target_rect, total_height, line.length);

        for (auto& drawable : line.drawables) {
            f32v2 size     = drawable.glyph->size * drawable.scaling;
            f32v2 position = f32v2(drawable.x_pos, current_y) + offsets + f32v2(target_rect.x, target_rect.y) + f32v2(0.0f, line.height - size.y);
            f32v4 uv_rect  = drawable.glyph->uv_rect;

            clip(clip_rect, position, size, uv_rect);

            batcher->add_sprite(drawable.texture, position, size, drawable.tint,
                                    { 255, 255, 255, 255 }, Gradient::NONE, depth, uv_rect);
        }

        current_y += line.height;
    }
}

/******************************************************\
 * Quick Wrap Draw                                    *
\******************************************************/

void hg::add_string_quick_wrap(SpriteBatcher* batcher, DrawableStringComponent component, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
    add_string_quick_wrap(batcher, &component, 1, target_rect, clip_rect, align, depth);
}

void hg::add_string_quick_wrap(SpriteBatcher* batcher, DrawableStringComponents components, ui32 num_components, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
    // We will populate these data points for drawing later.
    DrawableLines lines;
    f32 total_height = 0.0f;

    // Place the first line.
    lines.emplace_back(DrawableLine{0.0f, 0.0f, std::vector<DrawableGlyph>()});

    // TODO(Matthew): Can we make guesses as to the amount of drawables to reserve for a line? For amount of lines?

    for (ui32 component_idx = 0; component_idx < num_components; ++component_idx) {
        auto& component = components[component_idx];

        // Simplify property names.
        const char*  str    = component.str;
        FontInstance font   = component.props.font_instance;
        StringSizing sizing = component.props.sizing;
        colour4      tint   = component.props.tint;

        char start = font.owner->get_start();
        char end   = font.owner->get_end();

        // Process sizing into a simple scale factor.
        f32v2 scaling;
        f32   height;
        if (sizing.kind == StringSizingKind::SCALED) {
            scaling = sizing.scaling;
            height  = static_cast<f32>(font.height) * scaling.y;
        } else {
            scaling.x  = sizing.scale_x;
            scaling.y  = sizing.target_height / static_cast<f32>(font.height);
            height = sizing.target_height;
        }

        // auto hyphenate = [&]() {
        //     char   hyphen = '-';
        //     size_t index  = static_cast<size_t>(hyphen) - static_cast<size_t>(start);

        //     f32 character_width = font.glyphs[index].size.x * scaling.x;

        //     lines.back().drawables.emplace_back(DrawableGlyph{ &font.glyphs[index], lines.back().length, scaling, tint, font.texture });
        //     lines.back().length += character_width;
        // };

        // Gets set to true if we go out of the height of the rect.
        bool vertical_overflow = false;
        // Iterate over this component's string.
        for (ui32 i = 0; str[i] != '\0'; ++i) {
            char   character       = str[i];
            ui32 character_index = static_cast<ui32>(character) - static_cast<ui32>(start);

            // If character is a new line character, add a new line and go to next character.
            if (character == '\n') {
                total_height += lines.back().height;
                lines.emplace_back(DrawableLine{ 0.0f, height, std::vector<DrawableGlyph>() });

                continue;
            }

            // If character is unsupported, skip.
            if (character < start || character > end ||
                    !font.glyphs[character_index].supported) continue;

            // Determine character width after scaling.
            f32 character_width = font.glyphs[character_index].size.x * scaling.x;

            // Given we are about to add a character, make sure it fits on the line, if not, make
            // a new line and if the about-to-be-added character isn't a whitespace revisit it.
            if (lines.back().length + character_width > target_rect.z) {
                total_height += lines.back().height;
                lines.emplace_back(DrawableLine{ 0.0f, height, std::vector<DrawableGlyph>() });

                // Make sure to revisit this character if not whitespace.
                if (character != ' ') {
                    // hyphenate();
                    --i;
                }
                continue;
            }

            // If the line's height is less than the height of this font instance, and we're about to add a
            // glyph from this font instance, then the line's height needs setting to the font instances's.
            if (lines.back().height < height) {
                // If we have overflowed the rectangle with the new line height, break out instead.
                if (total_height + height > target_rect.w) {
                    vertical_overflow = true;
                    break;
                }
                lines.back().height = height;
            }

            // Add character to line for drawing.
            lines.back().drawables.emplace_back(DrawableGlyph{ &font.glyphs[character_index], lines.back().length, scaling, tint, font.texture });
            lines.back().length += character_width;
        }

        // If we have overflown vertically, break out of outer loop.
        if (vertical_overflow) break;
    }
    // Update the total height for last line.
    total_height += lines.back().height;

    f32 current_y = 0.0f;
    for (auto& line : lines) {
        f32v2 offsets = calculate_align_offset(align, target_rect, total_height, line.length);

        for (auto& drawable : line.drawables) {
            f32v2 size     = drawable.glyph->size * drawable.scaling;
            f32v2 position = f32v2(drawable.x_pos, current_y) + offsets + f32v2(target_rect.x, target_rect.y) + f32v2(0.0f, line.height - size.y);
            f32v4 uv_rect  = drawable.glyph->uv_rect;

            clip(clip_rect, position, size, uv_rect);

            batcher->add_sprite(drawable.texture, position, size, drawable.tint,
                                    { 255, 255, 255, 255 }, Gradient::NONE, depth, uv_rect);
        }

        current_y += line.height;
    }
}

/******************************************************\
 * Greedy Wrap Draw                                   *
\******************************************************/

void hg::add_string_greedy_wrap(SpriteBatcher* batcher, DrawableStringComponent component, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
    add_string_greedy_wrap(batcher, &component, 1, target_rect, clip_rect, align, depth);
}

void hg::add_string_greedy_wrap(SpriteBatcher* batcher, DrawableStringComponents components, ui32 num_components, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
    // We will populate these data points for drawing later.
    DrawableLines lines;
    f32 total_height = 0.0f;

    // Place the first line.
    lines.emplace_back(DrawableLine{0.0f, 0.0f, std::vector<DrawableGlyph>()});

    // TODO(Matthew): Can we make guesses as to the amount of drawables to reserve for a line? For amount of lines?

    for (ui32 i = 0; i < num_components; ++i) {
        auto& component = components[i];

        // Simplify property names.
        const char*  str    = component.str;
        FontInstance font   = component.props.font_instance;
        StringSizing sizing = component.props.sizing;
        colour4      tint   = component.props.tint;

        char start = font.owner->get_start();
        char end   = font.owner->get_end();

        // Process sizing into a simple scale factor.
        f32v2 scaling;
        f32   height;
        if (sizing.kind == StringSizingKind::SCALED) {
            scaling    = sizing.scaling;
            height = static_cast<f32>(font.height) * scaling.y;
        } else {
            scaling.x  = sizing.scale_x;
            scaling.y  = sizing.target_height / static_cast<f32>(font.height);
            height = sizing.target_height;
        }

        // We have to do per-word lookahead before adding any characters to the lines, these data points
        // enable us to do this.
        ui32 begin_index   = 0;
        ui32 current_index = 0;
        f32  wordLength    = 0.0f;

        // Useful functor to flush current word to line.
        auto flush_word_to_line = [&]() {
            while (begin_index != current_index) {
                // Get glyph index of character at string index.
                char   character      = str[begin_index];
                size_t character_index = static_cast<size_t>(character) - static_cast<size_t>(start);

                // Add character to line for drawing.
                lines.back().drawables.emplace_back(DrawableGlyph{ &font.glyphs[character_index], lines.back().length, scaling, tint, font.texture });

                // Determine character width after scaling & add to line length.
                f32 character_width = font.glyphs[character_index].size.x * scaling.x;
                lines.back().length += character_width;

                ++begin_index;
            }

            // Reset word length.
            wordLength = 0.0f;
        }; 

        // Gets set to true if we go out of the height of the rect.
        bool vertical_overflow = false;
        // Iterate over this component's string.
        for (; str[current_index] != '\0'; ++current_index) {
            char   character      = str[current_index];
            size_t character_index = static_cast<size_t>(character) - static_cast<size_t>(start);

            // If character is a new line character, add a new line and go to next character.
            if (character == '\n') {
                // But first, we need to flush the most recent word if it exists.
                flush_word_to_line();

                total_height += lines.back().height;
                lines.emplace_back(DrawableLine{ 0.0f, height, std::vector<DrawableGlyph>() });

                continue;
            }

            // If character is unsupported, skip.
            if (character < start || character > end ||
                    !font.glyphs[character_index].supported) continue;

            // Determine character width after scaling.
            f32 character_width = font.glyphs[character_index].size.x * scaling.x;

            // For characters on which we may break a line, flush the word so far
            // prematurely so that the breakable character can be handled correctly.
            if (character == ' ' || character == '-') {
                flush_word_to_line();
            }

            // Given we are about to add a character, make sure it fits on the line, if not, make
            // a new line and if the about-to-be-added character isn't a whitespace revisit it.
            if (lines.back().length + wordLength + character_width > target_rect.z) {
                total_height += lines.back().height;
                lines.emplace_back(DrawableLine{ 0.0f, height, std::vector<DrawableGlyph>() });

                // Skip whitespace at start of new line.
                if (str[begin_index] == ' ') {
                    ++begin_index;
                }
                continue;
            }

            // If the line's height is less than the height of this font instance, and we're about to add a
            // glyph from this font instance, then the line's height needs setting to the font instances's.
            if (lines.back().height < height) {
                // If we have overflowed the rectangle with the new line height, break out instead.
                if (total_height + height > target_rect.w) {
                    vertical_overflow = true;
                    break;
                }
                lines.back().height = height;
            }

            wordLength += character_width;
        }

        // If we have overflown vertically, break out of outer loop.
        if (vertical_overflow) break;

        // Flush any remaining word to line.
        flush_word_to_line();
    }
    // Update the total height for last line.
    total_height += lines.back().height;

    f32 current_y = 0.0f;
    for (auto& line : lines) {
        f32v2 offsets = calculate_align_offset(align, target_rect, total_height, line.length);

        for (auto& drawable : line.drawables) {
            f32v2 size         = drawable.glyph->size * drawable.scaling;
            f32v2 position     = f32v2(drawable.x_pos, current_y) + offsets + f32v2(target_rect.x, target_rect.y) + f32v2(0.0f, line.height - size.y);
            f32v4 uv_rect = drawable.glyph->uv_rect;

            clip(clip_rect, position, size, uv_rect);

            batcher->add_sprite(drawable.texture, position, size, drawable.tint,
                                    { 255, 255, 255, 255 }, Gradient::NONE, depth, uv_rect);
        }

        current_y += line.height;
    }
}



/******************************************************\
 * Minimum Raggedness Wrap Draw                       *
\******************************************************/

/**
 * @brief Data needed for each word in a text.
 */
struct Word {
    ui64 start   : 31;
    ui64 end     : 31;
    ui64 newline :  1;
    ui64 hyphen  :  1;
    f32  length;
};

// void hg::add_string_minimum_raggedness_wrap(SpriteBatcher* batcher, DrawableStringComponent component, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
//     add_string_minimum_raggedness_wrap(batcher, &component, 1, target_rect, clip_rect, align, depth);
// }

// void hg::add_string_minimum_raggedness_wrap(SpriteBatcher* batcher, DrawableStringComponents components, ui32 num_components, f32v4 target_rect, f32v4 clip_rect, TextAlign align, f32 depth) {
//     std::vector<std::vector<Word>> words;
//     words.resize(components.size());

//     // Split the string of each component into their costituent words in preparation for calculating the appropriate
//     // construction of lines to minimise the raggedness.
//     for (size_t i = 0; i < words.size(); ++i) {
//         auto& component = components[i];

//         // Simplify some component property names.
//         const char*  str    = component.str;
//         FontInstance font   = component.props.font_instance;
//         StringSizing sizing = component.props.sizing;

//         char start = font.owner->get_start();
//         char end   = font.owner->get_end();

//         // Process sizing into a simple scale factor.
//         f32v2 scaling;
//         f32   height;
//         if (sizing.kind == StringSizingKind::SCALED) {
//             scaling = sizing.scaling;
//             height  = static_cast<f32>(font.height) * scaling.y;
//         } else {
//             scaling.x = sizing.scale_x;
//             scaling.y = sizing.target_height / static_cast<f32>(font.height);
//             height    = sizing.target_height;
//         }

//         size_t begin_index   = 0;
//         size_t current_index = 0;

//         // Estabilsh the Word object we will use to building each
//         // word in the component string.
//         Word latest = {
//             0,
//             0,
//             0,
//             0,
//             0.0f
//         };

//         // Simple function to reset the latest Word object.
//         auto resetLatest = [&]() {
//             latest = {
//                 current_index + 1,
//                 current_index + 1,
//                 0,
//                 0,
//                 0.0f
//             };
//         }

//         for (; str[current_index] != '\0'; ++current_index) {
//             // Determine character at current point in string.
//             char   character      = str[current_index];
//             size_t character_index = static_cast<size_t>(character) - static_cast<size_t>(start);

//             // If character is unsupported, skip.
//             if (character < start || character > end ||
//                     !font.glyphs[character_index].supported) continue;

//             //// For any of the breakable characters, add the current word to the list of words
//             //// and prepare to accept a get the next word.
//             // If we are going to a new line, mark the next word as such.
//             if (character == '\n') {
//                 words[i].push_back(latest);

//                 resetLatest();

//                 latest.newline = 1;

//                 continue;
//             // If we are dealing with a hyphen, mark the next word as being hyphenated.
//             } else if (character == '-') {
//                 words[i].push_back(latest);

//                 resetLatest();

//                 latest.hyphen = 1;

//                 size_t index   = static_cast<size_t>('-') - static_cast<size_t>(start);
//                 latest.length += font.glyphs[index].size.x * scaling.x;

//                 continue;
//             // If we are just dealing with a new word, don't do anything extra.
//             } else if (character == ' ') {
//                 words[i].push_back(latest);

//                 resetLatest();

//                 continue;
//             }

//             // Increment end index of word.
//             ++latest.end;
//             // Determine character width after scaling and increment word length by it.
//             latest.length += font.glyphs[character_index].size.x * scaling.x;
//         }

//         //
//     }

//     std::vector<std::vector<f32>> displacements;
//     // TODO(Matthew): Can we make guesses as to the amount of drawables to reserve for a line? For amount of lines?


// }



//     // We will populate these data points for drawing later.
//     DrawableLines lines;
//     f32 total_height = 0.0f;

//     // Place the first line.
//     lines.emplace_back(DrawableLine{0.0f, 0.0f, std::vector<DrawableGlyph>()});

//     for (ui32 i = 0; i < num_components; ++i) {
//         auto& component = components[i];

//         // Simplify property names.
//         const char*  str    = component.str;
//         FontInstance font   = component.props.font_instance;
//         StringSizing sizing = component.props.sizing;
//         colour4      tint   = component.props.tint;

//         char start = font.owner->get_start();
//         char end   = font.owner->get_end();

//         // Process sizing into a simple scale factor.
//         f32v2 scaling;
//         f32   height;
//         if (sizing.kind == StringSizingKind::SCALED) {
//             scaling    = sizing.scaling;
//             height = static_cast<f32>(font.height) * scaling.y;
//         } else {
//             scaling.x  = sizing.scale_x;
//             scaling.y  = sizing.target_height / static_cast<f32>(font.height);
//             height = sizing.target_height;
//         }

//         // We have to do per-word lookahead before adding any characters to the lines, these data points
//         // enable us to do this.
//         ui32 begin_index   = 0;
//         ui32 current_index = 0;
//         f32  wordLength   = 0.0f;

//         // Useful functor to flush current word to line.
//         auto flush_word_to_line = [&]() {
//             while (begin_index != current_index) {
//                 // Get glyph index of character at string index.
//                 char   character      = str[begin_index];
//                 size_t character_index = static_cast<size_t>(character) - static_cast<size_t>(start);

//                 // Add character to line for drawing.
//                 lines.back().drawables.emplace_back(DrawableGlyph{ &font.glyphs[character_index], lines.back().length, scaling, tint, font.texture });

//                 // Determine character width after scaling & add to line length.
//                 f32 character_width = font.glyphs[character_index].size.x * scaling.x;
//                 lines.back().length += character_width;

//                 ++begin_index;
//             }

//             // Reset word length.
//             wordLength = 0.0f;
//         };

//         // Gets set to true if we go out of the height of the rect.
//         bool vertical_overflow = false;
//         // Iterate over this component's string.
//         for (; str[current_index] != '\0'; ++current_index) {
//             char   character      = str[current_index];
//             size_t character_index = static_cast<size_t>(character) - static_cast<size_t>(start);

//             // If character is a new line character, add a new line and go to next character.
//             if (character == '\n') {
//                 // But first, we need to flush the most recent word if it exists.
//                 flush_word_to_line();

//                 total_height += lines.back().height;
//                 lines.emplace_back(DrawableLine{ 0.0f, height, std::vector<DrawableGlyph>() });

//                 continue;
//             }

//             // If character is unsupported, skip.
//             if (character < start || character > end ||
//                     !font.glyphs[character_index].supported) continue;

//             // Determine character width after scaling.
//             f32 character_width = font.glyphs[character_index].size.x * scaling.x;

//             // For characters on which we may break a line, flush the word so far
//             // prematurely so that the breakable character can be handled correctly.
//             if (character == ' ' || character == '-') {
//                 flush_word_to_line();
//             }

//             // Given we are about to add a character, make sure it fits on the line, if not, make
//             // a new line and if the about-to-be-added character isn't a whitespace revisit it.
//             if (lines.back().length + wordLength + character_width > target_rect.z) {
//                 total_height += lines.back().height;
//                 lines.emplace_back(DrawableLine{ 0.0f, height, std::vector<DrawableGlyph>() });

//                 // Skip whitespace at start of new line.
//                 if (str[begin_index] == ' ') {
//                     ++begin_index;
//                 }
//                 continue;
//             }

//             // If the line's height is less than the height of this font instance, and we're about to add a
//             // glyph from this font instance, then the line's height needs setting to the font instances's.
//             if (lines.back().height < height) {
//                 // If we have overflowed the rectangle with the new line height, break out instead.
//                 if (total_height + height > target_rect.w) {
//                     vertical_overflow = true;
//                     break;
//                 }
//                 lines.back().height = height;
//             }

//             wordLength += character_width;
//         }

//         // If we have overflown vertically, break out of outer loop.
//         if (vertical_overflow) break;

//         // Flush any remaining word to line.
//         flush_word_to_line();
//     }
//     // Update the total height for last line.
//     total_height += lines.back().height;

//     f32 current_y = 0.0f;
//     for (auto& line : lines) {
//         f32v2 offsets = calculate_align_offset(align, target_rect, total_height, line.length);

//         for (auto& drawable : line.drawables) {
//             f32v2 size         = drawable.glyph->size * drawable.scaling;
//             f32v2 position     = f32v2(drawable.x_pos, current_y) + offsets + f32v2(target_rect.x, target_rect.y) + f32v2(0.0f, line.height - size.y);
//             f32v4 uv_rect = drawable.glyph->uv_rect;

//             clip(clip_rect, position, size, uv_rect);

//             batcher->add_sprite(drawable.texture, position, size, drawable.tint,
//                                      { 255, 255, 255, 255 }, Gradient::NONE, depth, uv_rect);
//         }

//         current_y += line.height;
//     }
// }
