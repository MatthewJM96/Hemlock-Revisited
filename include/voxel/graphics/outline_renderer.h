#ifndef __hemlock_voxel_graphics_outline_renderer_h
#define __hemlock_voxel_graphics_outline_renderer_h

#include "graphics/mesh.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        class ConditionalVoxelOutlineRenderer {
        public:
            ConditionalVoxelOutlineRenderer();
            ~ConditionalVoxelOutlineRenderer() { /* Empty. */ }

            /**
             * @brief Initialises voxel outline renderer.
             *
             * @param default_colour The default colour to render chunk outlines
             * with.
             */
            void init(colour4 default_colour);
            void dispose();

            void update(FrameTime time);
            void draw(FrameTime time);

            void set_default_colour(colour4 colour) { m_default_colour = colour; }
        protected:
            static hg::MeshHandles chunk_mesh_handles;

            /**
             * @brief Determines whether a chunk's outline should be rendered
             * and, if so, with what colour.
             * 
             * @param chunk The chunk to consider.
             * @return std::tuple<bool, colour4> First value is true if chunk's
             * outline should be rendered, false otherwise. Second value is the
             * colour with which to render the chunk.
             */
            virtual std::tuple<bool, colour4> should_render_chunk(hmem::Handle<hvox::Chunk> chunk) = 0;

            colour4 m_default_colour;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_graphics_outline_renderer_h
