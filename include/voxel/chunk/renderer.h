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

        struct PagedChunk {
            Chunk* chunk;

        };
        using PagedChunks = std::vector<PagedChunk>;

        struct ChunkRenderPage {
            PagedChunks chunks;
            ui32        voxel_count;
            GLuint      vbo;
            ui32        gpu_alloc_size;
            bool        dirty;
            ui32        first_dirtied_chunk_idx;
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

            ui32 page_size()       { return m_page_size;                                            };
            ui32 block_page_size() { return m_page_size * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE / 2; };

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

            /**
             * @brief Updates the buffer stored in the GPU
             * to reflect changes in the page's host buffer.
             *
             * @param page The page to process.
             */
            void process_page(ChunkRenderPage& page);

            ChunkRenderPages m_chunk_pages;

            ui32 m_page_size;
            ui32 m_max_unused_pages;

            ChunkRenderPages pages;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_renderer_h
