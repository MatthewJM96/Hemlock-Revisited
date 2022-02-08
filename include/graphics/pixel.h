#ifndef __hemlock_graphics_pixel_h
#define __hemlock_graphics_pixel_h

namespace hemlock {
    namespace graphics {
        namespace pixel {
            // TODO(Matthew): we could make some faster versions of convert_sdl_xxxx_8888_to_rgba_8888 by
            //                specialising for specific pixel formats in which we can do direct assignments.

            /**
             * @brief Converts an SDL_Surface that contains indexed pixel format data into a standard
             * RGBA 4-byte buffer.
             * 
             * @param sdl_surface The SDL surface to convert.
             * @param pixel_buffer The pixel buffer in which the resulting pixel information is stored.
             * @return True if the SDL surface was converted, false otherwise.
             */
            bool convert_sdl_indexed_to_rgba_8888(SDL_Surface* sdl_surface, CALLER_DELETE ui8*& pixel_buffer);

            /**
             * @brief Converts an SDL_Surface that contains an arbitrary 4-byte pixel format data into a
             * standard RGBA 4-byte buffer.
             * 
             * @param sdl_surface The SDL surface to convert.
             * @param pixel_buffer The pixel buffer in which the resulting pixel information is stored.
             * @return True if the SDL surface was converted, false otherwise.
             */
            bool convert_sdl_xxxx_8888_to_rgba_8888(SDL_Surface* sdl_surface, CALLER_DELETE ui8*& pixel_buffer);
        }
        namespace p = pixel;
    }
}
namespace hg  = hemlock::graphics;

#endif // __hemlock_graphics_pixel_h
