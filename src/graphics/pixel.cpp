#include "stdafx.h"

#include "graphics/pixel.h"

bool hg::p::convert_sdl_indexed_to_rgba_8888(SDL_Surface* sdl_surface, CALLER_DELETE ui8*& pixel_buffer) {
    if (sdl_surface == nullptr) return false;

    // Indexed SDL surfaces must be one byte per pixel.
    if (sdl_surface->format->BytesPerPixel != 1) return false;

    // Set up buffer.
    pixel_buffer = new ui8[4 * sdl_surface->w * sdl_surface->h];

    for (ui32 i = 0; i < static_cast<ui32>(sdl_surface->h); ++i) {
        for (ui32 j = 0; j < static_cast<ui32>(sdl_surface->w); ++j) {
            // Get index into our buffer and index into SDL pixel buffer.
            // In this case the SDL pixel buffer is actually a buffer of
            // indices into their colour palette.
            ui32 actual_pix_idx       = (i * sdl_surface->w)     + j;
            ui32 sdl_internal_pix_idx = (i * sdl_surface->pitch) + j;

            // Get index into SDL sruface's colour palette and grab the colour
            // of the pixel.
            ui8 colour_idx = reinterpret_cast<ui8*>(sdl_surface->pixels)[sdl_internal_pix_idx];
            SDL_Color* pix_colour = &sdl_surface->format->palette->colors[colour_idx];

            pixel_buffer[ actual_pix_idx * 4     ] = pix_colour->r;
            pixel_buffer[(actual_pix_idx * 4) + 1] = pix_colour->g;
            pixel_buffer[(actual_pix_idx * 4) + 2] = pix_colour->b;
            pixel_buffer[(actual_pix_idx * 4) + 3] = pix_colour->a;
        }
    }

    return true;
}

bool hg::p::convert_sdl_xxxx_8888_to_rgba_8888(SDL_Surface* sdl_surface, CALLER_DELETE ui8* pixel_buffer) {
    if (sdl_surface == nullptr) return false;

    // 8888 SDL surfaces must be four bytes per pixel.
    if (sdl_surface->format->BytesPerPixel != 4) return false;

    for (ui32 i = 0; i < static_cast<ui32>(glyph_surface->h); ++i) {
        for (ui32 j = 0; j < static_cast<ui32>(glyph_surface->w); ++j) {
            // Get index into our buffer and index into SDL pixel buffer.
            // In this case the SDL pixel buffer is actually a buffer of
            // indices into their colour palette.
            ui32 actual_pix_idx       = (i * glyph_surface->w)     +  j;
            ui32 sdl_internal_pix_idx = (i * glyph_surface->pitch) + (j * 4);

            // Get pixel information.
            ui32 pixel = *reinterpret_cast<ui32*>(
                &reinterpret_cast<ui8*>(glyph_surface->pixels)[sdl_internal_pix_idx]
            );

            /*
             * For each channel, apply the associated mask, shift and loss
             * factors to get the value of that channel regardless of the
             * particular 4-byte format being used.
             */

            ui32 red = pixel & glyph_surface->format->Rmask;
                 red = red  >> glyph_surface->format->Rshift;
                 red = red  << glyph_surface->format->Rloss;
            actual_pixels[ actual_pix_idx * 4     ] = red;

            ui32 green = pixel  & glyph_surface->format->Gmask;
                 green = green >> glyph_surface->format->Gshift;
                 green = green << glyph_surface->format->Gloss;
            actual_pixels[(actual_pix_idx * 4) + 1] = green;

            ui32 blue = pixel & glyph_surface->format->Bmask;
                 blue = blue >> glyph_surface->format->Bshift;
                 blue = blue << glyph_surface->format->Bloss;
            actual_pixels[(actual_pix_idx * 4) + 2] = blue;

            ui32 alpha = pixel  & glyph_surface->format->Amask;
                 alpha = alpha >> glyph_surface->format->Ashift;
                 alpha = alpha << glyph_surface->format->Aloss;
            actual_pixels[(actual_pix_idx * 4) + 3] = alpha;
        }
    }

    return true;
}
