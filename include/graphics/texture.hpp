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

            GLuint tex_id = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &tex_id);

            glTextureStorage2D(tex_id, 1, GL_RGBA8, dims.x, dims.y);
            glTextureSubImage2D(tex_id, 0, 0, 0, dims.x, dims.y, GL_RGBA, GL_UNSIGNED_BYTE, data);

            glTextureParameteri(tex_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(tex_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_S,     GL_REPEAT);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_T,     GL_REPEAT);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_R,     GL_REPEAT);

            delete[] data;

            // ui8* pixels = new ui8[dims.x * dims.y * 4];

            // glGetTextureImage(tex_id, 0, GL_RGBA, GL_UNSIGNED_BYTE, dims.x * dims.y * 4, pixels);

            // hio::img::png::save(filepath + "_rendered.png", reinterpret_cast<void*>(pixels), dims, format);

            return tex_id;
        }
    }
}
namespace hg  = hemlock::graphics;

#endif // __hemlock_graphics_texture_hpp
