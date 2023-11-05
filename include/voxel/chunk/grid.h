#ifndef __hemlock_voxel_chunk_grid_h
#define __hemlock_voxel_chunk_grid_h

#include "voxel/chunk/builder.h"
#include "voxel/coordinate_system.h"

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
            void init(
                hmem::WeakHandle<ChunkGrid>  self,
                ChunkBuilder*                chunk_builder,
                hmem::Handle<entt::registry> chunk_registry = nullptr
            );
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

            // ui32 chunks_in_render_distance() const {
            //     return m_chunks_in_render_distance;
            // }

            /**
             * @brief Loads a chunk. This entails saying it exists
             * and determining its neighbours - letting it and them
             * know of each other's existence.
             *
             * @param chunk_position The coords of the chunk to load.
             * @return True if the chunk was loaded into the grid,
             * false if the chunk is already loaded.
             */
            bool load_chunk(ChunkGridPosition chunk_position);
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
             * @brief Unloads a chunk, this entails ending all
             * pending tasks for this chunk and releasing memory
             * associated with it.
             *
             * NOTE: this is a non-blocking action, and the chunk
             * will only release memory once all active queries and
             * actions are completed.
             *
             * @param chunk_position The coords of the chunk to unload.
             * @return True if the chunk was unloaded, false otherwise.
             * False usually will mean that the chunk was not yet
             * existent, as if it is in any existing state some degree
             * of work will be done to unload it.
             */
            bool unload_chunk(ChunkGridPosition chunk_position);

            /**
             * @brief Returns a handle on the identified chunk
             * if it is held by the chunk grid.
             *
             * @param id The ID of the chunk to fetch.
             * @return entt::entity Entity identifying the
             * requested chunk, entt::null otherwise.
             */
            entt::entity chunk(ChunkID id);

            /**
             * @brief Returns a handle on the identified chunk
             * if it is held by the chunk grid.
             *
             * @param position The position of the chunk.
             * @return entt::entity Entity identifying the
             * requested chunk, entt::null otherwise.
             */
            entt::entity chunk(ChunkGridPosition position) {
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
            void establish_chunk_neighbours(entt::entity chunk);

            hmem::WeakHandle<ChunkGrid> m_self;

            ChunkBuilder* m_chunk_builder;

            entt::registry m_chunk_registry;
            Chunks         m_chunks;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_grid_h
