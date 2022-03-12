#ifndef __hemlock_voxel_chunk_renderer_h
#define __hemlock_voxel_chunk_renderer_h

#include "timing.h"
#include "graphics/mesh.h"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

        struct ChunkInstanceData {
            f32v3 translation, scaling;
        };

        struct ChunkRenderPage {
            ui32               voxel_count;
            ChunkInstanceData* instance_data;
            GLuint             vbo;
            bool               dirty;
        };
        using ChunkRenderPages = std::vector<ChunkRenderPage>;

        class ChunkRenderer {
        public:
            ChunkRenderer();
            ~ChunkRenderer() { /* Empty. */ }

            /**
             * @brief Initialises chunk renderer.
             *
             * @param page_size The number of instances to be stored per
             * page in units of half a block-volume of a chunk.
             * @param max_unused_pages The maximum number of pages that
             * will be retained that are not being used.
             */
            void init(ui32 page_size, ui32 max_unused_pages);
            void dispose();

            /**
             * @brief Set the page size.
             * 
             * @param page_size The number of instances to be stored per
             * page in units of half a block-volume of a chunk.
             */
            void set_page_size(ui32 page_size);

            void update(TimeData time);
            void draw(TimeData time);

            /**
             * @brief Renders a chunk into a sufficiently free page
             * for drawing.
             *
             * @param chunk 
             */
            void render_chunk(Chunk* chunk);
        protected:
            static hg::MeshHandles block_mesh_handles;

            /**
             * @brief Creates count number of new pages.
             * Returns the first of the newly created pages.
             *
             * @param count Number of pages to create.
             * @return ChunkRenderPage* Pointer to the first
             * page created by this call.
             */
            inline ChunkRenderPage* create_pages(ui32 count);

            ChunkRenderPages m_chunk_pages;

            ui32 m_page_size;
            ui32 m_max_unused_pages;

            ChunkRenderPages pages;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_renderer_h
