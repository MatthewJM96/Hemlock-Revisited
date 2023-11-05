#ifndef __hemlock_voxel_chunk_grid_h
#define __hemlock_voxel_chunk_grid_h

#include "algorithm/acs/graph/state.hpp"
#include "ecs/protected_component.hpp"
#include "timing.h"
#include "voxel/ai/navmesh/navmesh_manager.h"
#include "voxel/chunk/chunk.h"
#include "voxel/coordinate_system.h"
#include "voxel/graphics/renderer.h"
#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        using Chunks = std::unordered_map<ChunkID, entt::entity>;

        class ChunkGrid {
        public:
            ChunkGrid() {
                // Empty.
            }

            ~ChunkGrid() {
                // Empty.
            }

            /**
             * @brief
             */
            void init();
            /**
             * @brief
             */
            void dispose();

            // TODO(Matthew): here probably but need to decide how to enforce ideas of
            //                load/sim/render.
            // void set_load_distance(ui32 render_distance);
            // void set_render_distance(ui32 render_distance);

            // ui32 load_distance() const { return m_load_distance; }
            // ui32 render_distance() const { return m_render_distance; }

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
                ChunkGridPosition        chunk_position,
                hmem::WeakHandle<Chunk>* handle = nullptr
            );

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
            hmem::Handle<Chunk> chunk(ChunkGridPosition position) {
                return chunk(position.id);
            }

            const Chunks& chunks() const { return m_chunks; }

            /**
             * @brief Triggered whenever the render distance of this chunk grid
             * changes.
             */
            Event<RenderDistanceChangeEvent> on_render_distance_change;

            // TODO(Matthew): make these useful events.
            Event<> on_chunk_load;
            Event<> on_chunk_unload;
        protected:
            void establish_chunk_neighbours(hmem::Handle<Chunk> chunk);
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_grid_h
