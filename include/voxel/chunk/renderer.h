#ifndef __hemlock_voxel_chunk_renderer_h
#define __hemlock_voxel_chunk_renderer_h

#include "timing.h"
#include "graphics/mesh.h"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        class ChunkGrid;

        using AllPagedChunks = std::unordered_map<ChunkID, hmem::WeakHandle<Chunk>>;

        using PagedChunks = std::vector<ChunkID>;

        struct PagedChunkMetadata {
            ui32 page_idx;
            ui32 chunk_idx;
            ui32 last_voxel_count;
            bool paged;
        };
        using PagedChunksMetadata   = std::unordered_map<ChunkID, PagedChunkMetadata>;
        struct HandleAndID {
            hmem::WeakHandle<Chunk> handle;
            ChunkID                 id;
        };
        using PagedChunkQueue       = moodycamel::ConcurrentQueue<HandleAndID>;

        struct ChunkRenderPage {
            PagedChunks chunks;
            ui32        voxel_count;
            GLuint      vbo;
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

            ui32 page_size()       const { return m_page_size;                                            };
            ui32 block_page_size() const { return m_page_size * CHUNK_VOLUME / 2; };

            void update(TimeData time);
            void draw(TimeData time);

            /**
             * @brief Adds a chunk to the renderer, the
             * renderer registering with some of the
             * chunk's events to manage rendering of
             * the chunk.
             *
             * @param handle Weak handle on chunk to add.
             */
            void add_chunk(hmem::WeakHandle<Chunk> handle);
        protected:
            static hg::MeshHandles block_mesh_handles;

            Subscriber<>    handle_chunk_mesh_change;
            Subscriber<>    handle_chunk_unload;

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
             * @brief Puts a chunk in the first page starting
             * from the given index that has enough space for it.
             *
             * @param chunk The chunk to find a page for.
             * @param first_page_idx The index of the first page
             * to consider.
             */
            void put_chunk_in_page(hmem::Handle<Chunk> chunk, ui32 first_page_idx);

            /**
             * @brief Updates chunks, removing those that
             * are to be removed and then processing pages
             * that have dirtied chunks.
             */
            void process_pages();

            AllPagedChunks      m_all_paged_chunks;
            ChunkRenderPages    m_chunk_pages;
            PagedChunksMetadata m_chunk_metadata;
            PagedChunkQueue     m_chunk_removal_queue;
            PagedChunkQueue     m_chunk_dirty_queue;

            ui32 m_page_size;
            ui32 m_max_unused_pages;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_renderer_h
