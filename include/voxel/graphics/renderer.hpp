#ifndef __hemlock_voxel_graphics_renderer_hpp
#define __hemlock_voxel_graphics_renderer_hpp

#include "graphics/mesh.h"
#include "timing.h"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        template <ChunkDecorator... Decorations>
        struct Chunk;

        template <ChunkDecorator... Decorations>
        class ChunkGrid;

        template <ChunkDecorator... Decorations>
        using AllPagedChunks
            = std::unordered_map<ChunkID, hmem::WeakHandle<Chunk<Decorations...>>>;

        using PagedChunks = std::vector<ChunkID>;

        struct PagedChunkMetadata {
            ui32 page_idx;
            ui32 chunk_idx;
            ui32 on_gpu_offset;
            ui32 on_gpu_page_idx;
            ui32 on_gpu_voxel_count;
            bool dirty;
            bool paged;
        };

        using PagedChunksMetadata = std::unordered_map<ChunkID, PagedChunkMetadata>;

        template <ChunkDecorator... Decorations>
        struct HandleAndID {
            hmem::WeakHandle<Chunk<Decorations...>> handle;
            ChunkID                                 id;
        };

        template <ChunkDecorator... Decorations>
        using PagedChunkQueue
            = moodycamel::ConcurrentQueue<HandleAndID<Decorations...>>;

        struct ChunkRenderPage {
            PagedChunks chunks;
            ui32        voxel_count;
            GLuint      vbo;
            bool        dirty;
            ui32        first_dirtied_chunk_idx;
        };

        using ChunkRenderPages = std::vector<ChunkRenderPage*>;

        template <ChunkDecorator... Decorations>
        class ChunkRenderer {
        public:
            using _Chunk           = Chunk<Decorations...>;
            using _ChunkGrid       = ChunkGrid<Decorations...>;
            using _AllPagedChunks  = AllPagedChunks<Decorations...>;
            using _HandleAndID     = HandleAndID<Decorations...>;
            using _PagedChunkQueue = PagedChunkQueue<Decorations...>;

            ChunkRenderer();

            ~ChunkRenderer() {
                // Empty.
            }

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

            ui32 page_size() const { return m_page_size; };

            ui32 block_page_size() const { return m_page_size * CHUNK_VOLUME / 2; };

            void update(FrameTime time);
            void draw(FrameTime time);

            /**
             * @brief Adds a chunk to the renderer, the
             * renderer registering with some of the
             * chunk's events to manage rendering of
             * the chunk.
             *
             * @param handle Weak handle on chunk to add.
             */
            void add_chunk(hmem::WeakHandle<_Chunk> handle);
        protected:
            static hg::MeshHandles block_mesh_handles;

            Subscriber<> handle_chunk_mesh_change;
            Subscriber<> handle_chunk_unload;

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
             * @param chunk_id The ID of the chunk to find a page for.
             * @param instance_count The number of instances representing the chunk.
             * @param first_page_idx The index of the first page
             * to consider.
             */
            void put_chunk_in_page(
                ChunkID chunk_id, ui32 instance_count, ui32 first_page_idx
            );

            /**
             * @brief Updates chunks, removing those that
             * are to be removed and then processing pages
             * that have dirtied chunks.
             */
            void process_pages();

            _AllPagedChunks     m_all_paged_chunks;
            ChunkRenderPages    m_chunk_pages;
            PagedChunksMetadata m_chunk_metadata;
            _PagedChunkQueue    m_chunk_removal_queue;
            _PagedChunkQueue    m_chunk_dirty_queue;

            ui32 m_page_size;
            ui32 m_max_unused_pages;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/graphics/renderer.inl"

#endif  // __hemlock_voxel_graphics_renderer_hpp
