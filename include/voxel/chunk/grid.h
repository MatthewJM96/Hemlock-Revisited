#ifndef __hemlock_voxel_chunk_grid_h
#define __hemlock_voxel_chunk_grid_h

#include "timing.h"
#include "voxel/coordinate_system.h"
#include "voxel/chunk.h"
#include "voxel/chunk/load_task.hpp"
#include "voxel/chunk/renderer.h"

namespace hemlock {
    namespace voxel {

        // TODO(Matthew): Does page size want to be made a run-time thing,
        //                as it may be nice to base this on view distance.
        using ChunkAllocator = hmem::PagedAllocator<Chunk, 4 * 4 * 4, 3>;

        struct Chunk;
        // TODO(Matthew): We should add support for LOD, different generation stages, disabling meshing etc.

        using Chunks = std::unordered_map<ChunkID, hmem::Handle<Chunk>>;

        using QueriedChunkState       = std::pair<bool, bool>;
        using QueriedChunkPendingTask = std::pair<bool, bool>;

        using ChunkLoadTaskListBuilder = Delegate<hthread::ThreadWorkflowTasksView<ChunkLoadTaskContext>(hmem::WeakHandle<Chunk>, hmem::WeakHandle<ChunkGrid>)>;

        class ChunkGrid {
        public:
            ChunkGrid()  { /* Empty. */ }
            ~ChunkGrid() { /* Empty. */ }

            /**
             * @brief Initialises the chunk grid and the
             * underlying thread pool.
             *
             * @param self A weak handle on this grid instance.
             * @param thread_count The number of threads
             * that the grid can use for loading tasks.
             * @param chunk_load_dag The DAG of the workflow
             * to do chunk loading with.
             * @param chunk_load_task_list_builder The builder
             * used to generate tasks for loading chunks.
             */
            void init( hmem::WeakHandle<ChunkGrid> self,
                                              ui32 thread_count,
                        thread::ThreadWorkflowDAG* chunk_load_dag,
                          ChunkLoadTaskListBuilder chunk_load_task_list_builder );
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
            void update(TimeData time);
            /**
             * @brief Draw loop for chunks.
             *
             * @param time The time data for the frame.
             */
            void draw(TimeData time);

            // TODO(Matthew): move this out of here. we should look
            //                at Vulkan for how we might better architect drawing.
            /**
             * @brief Draw chunk grid.
             */
            void draw_grid();

            /**
             * @brief Suspends chunk tasks. This is a hammer, but
             * for testing it can definitely be useful. Probably
             * don't ever call this in practise.
             */
            void suspend_chunk_tasks() { m_chunk_load_thread_pool.suspend(); }
            /**
             * @brief Resumes chunk tasks. No consequences for
             * calling this when not already suspended.
             */
            void resume_chunk_tasks()  { m_chunk_load_thread_pool.resume();  }

            ChunkRenderer* renderer() { return &m_renderer; }

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
            bool load_from_scratch_chunks( ChunkGridPosition* chunk_positions,
                                                         ui32 chunk_count );

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
             * @brief Loads a chunk, preloading it if it has not yet
             * been designated as existing.
             *
             * @param chunk_position The coords of the chunk to load.
             * @return True if the chunk's load task was queued, false
             * otherwise. False usually will mean that the chunk was
             * either not yet preloaded, or at least already in a loaded
             * state.
             */
            bool load_from_scratch_chunk_at(ChunkGridPosition chunk_position);
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
            bool unload_chunk_at(ChunkGridPosition chunk_position, hmem::WeakHandle<Chunk>* handle = nullptr);

            /**
             * @brief Queries the state of the chunk at the given
             * position. The requirement verified here is that
             * the so-positioned chunk is at the very least in
             * the specified state; "later" states shall also
             * satisfy the requirement here.
             *
             * @param chunk_position The position of the chunk
             * to query.
             * @param required_minimum_state The minimum state
             * required of the chunk.
             * @return [true, true] if the chunk is at least in the
             * required state, [true, false] if the chunk exists
             * but does not satisfy the state requirement,
             * [false, false] if the chunk does not exist. Note:
             * [false, true] should never occur and represents
             * invalid query processing.
             */
            QueriedChunkState query_chunk_state(ChunkGridPosition chunk_position, ChunkState required_minimum_state);
            /**
             * @brief Queries the state of the chunk. The
             * requirement verified here is that the chunk is
             * at the very least in the specified state;
             * "later" states shall also satisfy the
             * requirement here.
             *
             * @param chunk The chunk to query
             * @param required_minimum_state The minimum state
             * required of the chunk.
             * @return [true, true] if the chunk is at least in the
             * required state, [true, false] if the chunk exists
             * but does not satisfy the state requirement,
             * [false, false] if the chunk does not exist. Note:
             * [false, true] should never occur and represents
             * invalid query processing.
             */
            QueriedChunkState query_chunk_state(hmem::Handle<Chunk> chunk, ChunkState required_minimum_state);

            /**
             * @brief Queries the pending task of the chunk
             * at the given position. The requirement verified
             * here is that the so-positioned chunk is at the
             * very least pending going into the specified
             * task; "later" tasks shall also satisfy the
             * requirement here as that implies the task
             * has already been achieved.
             *
             * @param chunk_position The position of the chunk
             * to query.
             * @param required_minimum_pending_task The minimum
             * task required of the chunk.
             * @return [true, true] if the chunk is at least
             * pending the required task, [true, false] if the
             * chunk exists but does not satisfy the pending
             * task requirement, [false, false] if the chunk
             * does not exist. Note: [false, true] should
             * never occur and represents invalid query
             * processing.
             */
            QueriedChunkPendingTask query_chunk_pending_task(ChunkGridPosition chunk_position, ChunkLoadTaskKind required_minimum_pending_task);
            /**
             * @brief Queries the pending task of the chunk
             *  The requirement verified here is that the
             * chunk is at the very least pending going into
             * the specified task; "later" tasks shall also
             * satisfy the requirement here as that implies
             * the task has already been achieved.
             *
             * @param chunk The chunk to query
             * @param required_minimum_pending_task The minimum
             * task required of the chunk.
             * @return [true, true] if the chunk is at least
             * pending the required task, [true, false] if the
             * chunk exists but does not satisfy the pending
             * task requirement, [false, false] if the chunk
             * does not exist. Note: [false, true] should
             * never occur and represents invalid query
             * processing.
             */
            QueriedChunkPendingTask query_chunk_pending_task(hmem::Handle<Chunk> chunk, ChunkLoadTaskKind required_minimum_pending_task);

            /**
             * @brief Queries the state of the neighbours of
             * the chunk at the given position. The requirement
             * verified here is that the neighbours of the
             * so-positioned chunk are at the very least in
             * the specified state; "later" states shall also
             * satisfy the requirement here.
             *
             * @param chunk_position The position of the chunk
             * whose neighbours are to be queried.
             * @param required_minimum_state The minimum state
             * required of the chunks.
             * @return [true, true] if the chunks are at least in the
             * required state, [true, false] if the chunk whose
             * neighbours we are querying exists but its neighbours
             * do not satisfy the state requirement, [false, false]
             * if the chunk whose neighbours we are querying does
             * not exist. Note: [false, true] should never occur
             * and represents invalid query processing.
             */
            QueriedChunkState query_all_neighbour_states(ChunkGridPosition chunk_position, ChunkState required_minimum_state);
            /**
             * @brief Queries the state of the neighbours of
             * the chunk. The requirement verified here is
             * that the neighbours of the chunk are at the
             * very least in the specified state; "later"
             * states shall also satisfy the requirement here.
             *
             * @param chunk The chunk whose neighbours are to
             * be queried.
             * @param required_minimum_state The minimum state
             * required of the chunks.
             * @return [true, true] if the chunks are at least in the
             * required state, [true, false] if the chunk whose
             * neighbours we are querying exists but its neighbours
             * do not satisfy the state requirement, [false, false]
             * if the chunk whose neighbours we are querying does
             * not exist. Note: [false, true] should never occur
             * and represents invalid query processing.
             */
            QueriedChunkState query_all_neighbour_states(hmem::Handle<Chunk> chunk, ChunkState required_minimum_state);

            /**
             * @brief Queries the state of the chunk at the given
             * position. The requirement verified here is that
             * the so-positioned chunk is exactly in the specified
             * state; "later" states shall not satisfy the
             * requirement here.
             *
             * @param chunk_position The position of the chunk
             * to query.
             * @param required_state The state required
             * of the chunk.
             * @return [true, true] if the chunk is in the required
             * state, [true, false] if the chunk exists but does
             * not satisfy the state requirement, [false, false]
             * if the chunk does not exist. Note: [false, true]
             * should never occur and represents invalid query
             * processing.
             */
            QueriedChunkState query_chunk_exact_state(ChunkGridPosition chunk_position, ChunkState required_state);
            /**
             * @brief Queries the state of the chunk. The
             * requirement verified here is that the chunk
             * is exactly in the specified state; "later"
             * states shall not satisfy the requirement
             * here.
             *
             * @param chunk The chunk to query
             * @param required_state The required state
             * required of the chunk.
             * @return [true, true] if the chunk is in the
             * required state, [true, false] if the chunk exists
             * but does not satisfy the state requirement,
             * [false, false] if the chunk does not exist. Note:
             * [false, true] should never occur and represents
             * invalid query processing.
             */
            QueriedChunkState query_chunk_exact_state(hmem::Handle<Chunk> chunk, ChunkState required_state);

            /**
             * @brief Queries the pending task of the chunk
             * at the given position. The requirement verified
             * here is that the so-positioned chunk is exactly
             * in the specified state; "later" states shall
             * not satisfy the requirement here.
             *
             * @param chunk_position The position of the chunk
             * to query.
             * @param required_pending_task The task required
             * to be pending for the chunk.
             * @return [true, true] if the chunk is pending
             * the required task, [true, false] if the chunk
             * exists but does not satisfy the pending task
             * requirement, [false, false] if the chunk does
             * not exist. Note: [false, true] should never
             * occur and represents invalid query processing.
             */
            QueriedChunkPendingTask query_chunk_exact_pending_task(ChunkGridPosition chunk_position, ChunkLoadTaskKind required_pending_task);
            /**
             * @brief Queries the pending task of the chunk
             * The requirement verified here is that the
             * chunk is exactly in the specified state;
             * "later" states shall not satisfy the
             * requirement here.
             *
             * @param chunk The chunk to query
             * @param required_pending_task The task
             * required of the chunk.
             * @return [true, true] if the chunk is pending
             * the required task, [true, false] if the chunk
             * exists but does not satisfy the pending task
             * requirement, [false, false] if the chunk does
             * not exist. Note: [false, true] should never
             * occur and represents invalid query processing.
             */
            QueriedChunkPendingTask query_chunk_exact_pending_task(hmem::Handle<Chunk> chunk, ChunkLoadTaskKind required_pending_task);

            /**
             * @brief Queries the state of the neighbours of
             * the chunk at the given position. The requirement
             * verified here is that the neighbours of the
             * so-positioned chunk are exactly in the specified
             * state; "later" states shall not satisfy the
             * requirement here.
             *
             * @param chunk_position The position of the chunk
             * whose neighbours are to be queried.
             * @param required_state The state required of the
             * neighbouring chunks.
             * @return [true, true] if the neighbouring chunks
             * are in the required state, [true, false] if the
             * chunk whose neighbours we are querying exists but
             * its neighbours do not satisfy the state requirement,
             * [false, false] if the chunk whose neighbours we are
             * querying does not exist. Note: [false, true] should
             * never occur and represents invalid query processing.
             */
            QueriedChunkState query_all_neighbour_exact_states(ChunkGridPosition chunk_position, ChunkState required_state);
            /**
             * @brief Queries the state of the neighbours of
             * the chunk. The requirement verified here is
             * that the neighbours of the chunk are exactly
             * in the specified state; "later" states shall
             * not satisfy the requirement here.
             *
             * @param chunk The chunk whose neighbours are to
             * be queried.
             * @param required_state The state required of the
             * neighbouring chunks.
             * @return [true, true] if the neighbouring chunks are
             * in the required state, [true, false] if the chunk
             * whose neighbours we are querying exists but its
             * neighbours do not satisfy the state requirement,
             * [false, false] if the chunk whose neighbours we are
             * querying does not exist. Note: [false, true] should
             * never occur and represents invalid query processing.
             */
            QueriedChunkState query_all_neighbour_exact_states(hmem::Handle<Chunk> chunk, ChunkState required_state);

            /**
             * @brief Returns a handle on the identified chunk
             * if it is held by the chunk grid.
             *
             * @param id The ID of the chunk to fetch.
             * @return hmem::Handle<Chunk> Handle on the
             * requested chunk, nullptr otherwise.
             */
            hmem::Handle<Chunk> chunk(ChunkID id);
            /**
             * @brief Returns a handle on the identified chunk
             * if it is held by the chunk grid.
             *
             * @param position The position of the chunk.
             * @return hmem::Handle<Chunk> Handle on the
             * requested chunk, nullptr otherwise.
             */
            hmem::Handle<Chunk> chunk(ChunkGridPosition position) { return chunk(position.id); }
        protected:
            void establish_chunk_neighbours(hmem::Handle<Chunk> chunk);

            ChunkLoadTaskListBuilder   build_load_tasks;

            thread::ThreadPool<ChunkLoadTaskContext>        m_chunk_load_thread_pool;
            thread::ThreadWorkflow<ChunkLoadTaskContext>    m_chunk_load_workflow;

            ChunkAllocator                          m_chunk_allocator;
            hmem::Handle<ChunkBlockPager>           m_block_pager;
            hmem::Handle<ChunkInstanceDataPager>    m_instance_data_pager;

            ChunkRenderer m_renderer;

            Chunks m_chunks;

            hmem::WeakHandle<ChunkGrid> m_self;

            // TODO(Matthew): MOVE IT
            GLuint m_grid_vao, m_grid_vbo;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_grid_h
