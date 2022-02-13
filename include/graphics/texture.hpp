#ifndef __hemlock_graphics_texture_hpp
#define __hemlock_graphics_texture_hpp

#include "io/image.h"

namespace hemlock {
    namespace graphics {
        GLuint load_texture(const std::string& filepath) {
            void*   data = nullptr;
            ui32v2 dims = ui32v2{0};
            hio::img::PixelFormat format;
            if (!hio::img::png::load(filepath, data, dims, format)) return 0;

            GLuint tex_id;
            glGenTextures(1, &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dims.x, dims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,     GL_REPEAT);

            glBindTexture(GL_TEXTURE_2D, 0);

            delete[] data;

            // ui8* pixels = new ui8[dims.x * dims.y * 4];

            // glBindTexture(GL_TEXTURE_2D, tex_id);
            // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            // glBindTexture(GL_TEXTURE_2D, 0);

            // hio::img::png::save(filepath + "_rendered.png", reinterpret_cast<void*>(pixels), dims, format);

            return tex_id;
        }
    }
}
namespace hg  = hemlock::graphics;

#endif // __hemlock_graphics_texture_hpp
