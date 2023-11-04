#ifndef __hemlock_voxel_chunk_grid_hpp
#define __hemlock_voxel_chunk_grid_hpp

#include "timing.h"
#include "voxel/chunk/chunk.hpp"
#include "voxel/chunk/decorator/decorator.hpp"
#include "voxel/coordinate_system.h"
#include "voxel/graphics/renderer.hpp"
#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        // TODO(Matthew): Does page size want to be made a run-time thing,
        //                as it may be nice to base this on view distance.
        template <ChunkDecorator... Decorations>
        using ChunkAllocator
            = hmem::PagedAllocator<Chunk<Decorations...>, 4 * 4 * 4, 3>;

        template <ChunkDecorator... Decorations>
        using Chunks = std::unordered_map<ChunkID, hmem::Handle<Chunk<Decorations...>>>;

        template <ChunkDecorator... Decorations>
        using ChunkTaskBuilder = Delegate<ChunkTask<Decorations...>*(void)>;

        template <ChunkDecorator... Decorations>
        class ChunkGrid {
        public:
            using _ChunkAllocator   = ChunkAllocator<Decorations...>;
            using _Chunk            = Chunk<Decorations...>;
            using _Chunks           = Chunks<Decorations...>;
            using _ChunkGrid        = ChunkGrid<Decorations...>;
            using _ChunkTaskBuilder = ChunkTaskBuilder<Decorations...>;

            ChunkGrid();

            ~ChunkGrid() {
                // Empty.
            }

            /**
             * @brief Initialises the chunk grid and the
             * underlying thread pool.
             *
             * @param self A weak handle on this grid instance.
             * @param render_distance The render distance the grid starts with.
             * @param thread_count The number of threads
             * that the grid can use for loading tasks.
             * @param build_load_or_generate_task Builder that returns
             * a valid task to load a chunk from disk if present or
             * otherwise generate it.
             * @param build_mesh_task Builder that returns a valid
             * task to mesh a chunk.
             * @param build_navmesh_task Builder that returns a valid
             * task to navmesh a chunk.
             */
            void init(
                hmem::WeakHandle<_ChunkGrid> self,
                ui32                         render_distance,
                ui32                         thread_count,
                _ChunkTaskBuilder            build_load_or_generate_task,
                _ChunkTaskBuilder            build_mesh_task,
                _ChunkTaskBuilder*           build_navmesh_task = nullptr
            );
            /**
             * @brief Disposes of the chunk grid, ending
             * the tasks on the thread pool and unloading
             * all chunks.
             */
            void dispose();

            /**
             * @brief Update loop for chunks.
             *
             * @param time The time data for the frame.
             */
            void update(FrameTime time);
            /**
             * @brief Draw loop for chunks.
             *
             * @param time The time data for the frame.
             */
            void draw(FrameTime time);

            void set_render_distance(ui32 render_distance);

            ui32 render_distance() const { return m_render_distance; }

            ui32 chunks_in_render_distance() const {
                return m_chunks_in_render_distance;
            }

            /**
             * @brief Suspends chunk tasks. This is a hammer, but
             * for testing it can definitely be useful. Probably
             * don't ever call this in practise.
             */
            void suspend_chunk_tasks() { m_thread_pool.suspend(); }

            /**
             * @brief Resumes chunk tasks. No consequences for
             * calling this when not already suspended.
             */
            void resume_chunk_tasks() { m_thread_pool.resume(); }

            ChunkRenderer<Decorations...>* renderer() { return &m_renderer; }

            /**
             * @brief Loads chunks with the assumption none specified
             * have even been preloaded. This is useful as it assures
             * all preloading is done before any loading so that there
             * is no need for corrective load tasks later for adjoining
             * chunks etc.
             *
             * @param chunk_positions The array of chunk coords for which
             * to load chunks.
             * @param chunk_count The number of chunks to load.
             * @return True if all chunks have got to the point of their
             * load tasks being queued in a valid state, false if any single
             * chunk did not.
             */
            bool load_chunks(ChunkGridPosition* chunk_positions, ui32 chunk_count);

            /**
             * @brief Preloads a chunk, this entails saying it exists
             * and determining its neighbours - letting it and them
             * know of each other's existence.
             *
             * @param chunk_position The coords of the chunk to preload.
             * @return True if the chunk was preloaded, false otherwise.
             * False usually will mean that the chunk was at least already
             * in a preloaded state.
             */
            bool preload_chunk_at(ChunkGridPosition chunk_position);
            /**
             * @brief Loads a chunk, this entails queueing the
             * provided workflow to run.
             *
             * @param chunk_position The coords of the chunk to load.
             * @return True if the chunk's load task was queued, false
             * otherwise. False usually will mean that the chunk was
             * either not yet preloaded, or at least already in a loaded
             * state.
             */
            bool load_chunk_at(ChunkGridPosition chunk_position);
            /**
             * @brief Unloads a chunk, this entails ending all
             * pending tasks for this chunk and releasing memory
             * associated with it.
             *
             * NOTE: this is a non-blocking action, and the chunk
             * will only release memory once all active queries and
             * actions are completed.
             *
             * @param chunk_position The coords of the chunk to unload.
             * @param handle Optional weak handle into which the chunk
             * will be placed. Useful to detect when the chunk is finally
             * fully released.
             * @return True if the chunk was unloaded, false otherwise.
             * False usually will mean that the chunk was not yet
             * existent, as if it is in any existing state some degree
             * of work will be done to unload it.
             */
            bool unload_chunk_at(
                ChunkGridPosition         chunk_position,
                hmem::WeakHandle<_Chunk>* handle = nullptr
            );

            /**
             * @brief Returns a handle on the identified chunk
             * if it is held by the chunk grid.
             *
             * @param id The ID of the chunk to fetch.
             * @return hmem::Handle<Chunk> Handle on the
             * requested chunk, nullptr otherwise.
             */
            hmem::Handle<_Chunk> chunk(ChunkID id);

            /**
             * @brief Returns a handle on the identified chunk
             * if it is held by the chunk grid.
             *
             * @param position The position of the chunk.
             * @return hmem::Handle<Chunk> Handle on the
             * requested chunk, nullptr otherwise.
             */
            hmem::Handle<_Chunk> chunk(ChunkGridPosition position) {
                return chunk(position.id);
            }

            const _Chunks& chunks() const { return m_chunks; }

            /**
             * @brief Triggered whenever the render distance of this chunk grid
             * changes.
             */
            Event<RenderDistanceChangeEvent> on_render_distance_change;
        protected:
            void establish_chunk_neighbours(hmem::Handle<_Chunk> chunk);

            Delegate<void(Sender)>                   handle_chunk_load;
            Delegate<bool(Sender, BlockChangeEvent)> handle_block_change;

            _ChunkTaskBuilder m_build_load_or_generate_task, m_build_mesh_task,
                m_build_navmesh_task;
            thread::ThreadPool<ChunkTaskContext> m_thread_pool;

            _ChunkAllocator m_chunk_allocator;

            hmem::Handle<ChunkBlockPager>        m_block_pager;
            hmem::Handle<ChunkInstanceDataPager> m_instance_pager;
            hmem::Handle<ai::ChunkNavmeshPager>  m_navmesh_pager;

            // TODO(Matthew): move this out of here?
            ChunkRenderer<Decorations...> m_renderer;
            ui32 m_render_distance, m_chunks_in_render_distance;

            _Chunks m_chunks;

            hmem::WeakHandle<_ChunkGrid> m_self;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#include "voxel/chunk/grid.inl"

#endif  // __hemlock_voxel_chunk_grid_hpp
