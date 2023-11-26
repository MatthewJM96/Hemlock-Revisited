#ifndef __hemlock_graphics_texture_hpp
#define __hemlock_graphics_texture_hpp

#include "io/image.h"

namespace hemlock {
    namespace graphics {
        GLuint load_texture(const std::string& filepath) {
            ui8*                  data = nullptr;
            ui32v2                dims = ui32v2{ 0 };
            hio::img::PixelFormat format;
            if (!hio::img::png::load(filepath, data, dims, format)) return 0;

            GLuint tex_id = 0;
#if !defined(HEMLOCK_OS_MAC)
            glCreateTextures(GL_TEXTURE_2D, 1, &tex_id);

            GLsizei max_mipmaps
                = 1
                  + static_cast<GLsizei>(
                      glm::log2(static_cast<f32>(glm::max(dims.x, dims.y)))
                  );

            // TODO(Matthew): use format to correctly store sub image.

            glTextureStorage2D(tex_id, max_mipmaps, GL_RGBA8, dims.x, dims.y);
            glTextureSubImage2D(
                tex_id, 0, 0, 0, dims.x, dims.y, GL_RGBA, GL_UNSIGNED_BYTE, data
            );

            glTextureParameteri(tex_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(tex_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_R, GL_REPEAT);

            glGenerateTextureMipmap(tex_id);

            // ui8* pixels = new ui8[dims.x * dims.y * 4];

            // glGetTextureImage(tex_id, 0, GL_RGBA, GL_UNSIGNED_BYTE, dims.x * dims.y
            // * 4, pixels);

            // hio::img::png::save(filepath + "_rendered.png",
            // reinterpret_cast<void*>(pixels), dims, format);
#else   // !defined(HEMLOCK_OS_MAC)
        // Create the texture we will put each glyph into.
            glGenTextures(1, &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);

            GLsizei max_mipmaps
                = 1
                  + static_cast<GLsizei>(
                      glm::log2(static_cast<f32>(glm::max(dims.x, dims.y)))
                  );

            // TODO(Matthew): use format to correctly store sub image.

            GLsizei width  = dims.x;
            GLsizei height = dims.y;
            for (GLint level = 0; level < max_mipmaps; ++level) {
                glTexImage2D(
                    GL_TEXTURE_2D,
                    max_mipmaps,
                    GL_RGBA8,
                    1,
                    width,
                    height,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    nullptr
                );

                if (width == 1 && height == 1) {
                    max_mipmaps = level + 1;
                }

                width  = std::max(1, width / 2);
                height = std::max(1, height / 2);
            }

            glTexSubImage2D(
                GL_TEXTURE_2D, 0, 0, 0, dims.x, dims.y, GL_RGBA, GL_UNSIGNED_BYTE, data
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

            glGenerateMipmap(GL_TEXTURE_2D);

            // ui8* pixels = new ui8[dims.x * dims.y * 4];

            // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, dims.x *
            // dims.y
            // * 4, pixels);

            // hio::img::png::save(filepath + "_rendered.png",
            // reinterpret_cast<void*>(pixels), dims, format);

            glBindTexture(GL_TEXTURE_2D, 0);
#endif  // !defined(HEMLOCK_OS_MAC)

            delete[] data;

            return tex_id;
        }
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_texture_hpp
