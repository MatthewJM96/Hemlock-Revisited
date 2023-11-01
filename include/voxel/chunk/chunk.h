#ifndef __hemlock_voxel_chunk_chunk_h
#define __hemlock_voxel_chunk_chunk_h

#include "graphics/mesh.h"
#include "timing.h"
#include "voxel/ai/navmesh.hpp"
#include "voxel/block.hpp"
#include "voxel/chunk/constants.hpp"
#include "voxel/chunk/events.hpp"
#include "voxel/chunk/state.hpp"
#include "voxel/coordinate_system.h"
#include "voxel/graphics/mesh/instance_manager.h"
#include "voxel/task.hpp"

namespace hemlock {
    namespace voxel {
        /**
         * @brief
         */
        struct Chunk {
            Chunk();
            ~Chunk();

            void init(
                hmem::WeakHandle<Chunk>              self,
                hmem::Handle<ChunkBlockPager>        block_pager,
                hmem::Handle<ChunkInstanceDataPager> instance_data_pager
            );

            void update(FrameTime);

            ChunkID id() const { return position.id; }

            ChunkGridPosition position;
            Neighbours        neighbours;

            std::shared_mutex blocks_mutex;
            Block*            blocks;

            std::shared_mutex navmesh_mutex;
            ai::ChunkNavmesh  navmesh;

            ChunkInstanceManager instance;

            std::atomic<LODLevel>   lod_level;
            std::atomic<ChunkState> generation, meshing, mesh_uploading,
                bulk_navmeshing, navmeshing;

            struct {
                std::atomic<ChunkState> right, top, front, above_left, above_right,
                    above_front, above_back, above_and_across_left,
                    above_and_across_right, above_and_across_front,
                    above_and_across_back;
            } navmesh_stitch;

            CancellableEvent<BlockChangeEvent>     on_block_change;
            CancellableEvent<BulkBlockChangeEvent> on_bulk_block_change;

            // NOTE(Matthew): These events, at least on_mesh_change, can be
            //                called from multiple threads. Events are NOT
            //                thread-safe. Our one guarantee is not really
            //                a full guarantee but should hold true: only
            //                one thread that could make a state change
            //                should be processing a task regarding this
            //                chunk at any point in time. If this fails
            //                to hold up, then we could easily get race
            //                conditions inside the events.
            Event<>               on_load;
            Event<>               on_mesh_change;
            Event<>               on_navmesh_change;
            Event<LODChangeEvent> on_lod_change;
            Event<>               on_unload;
        protected:
            void init_events(hmem::WeakHandle<Chunk> self);

            hmem::Handle<ChunkBlockPager> m_block_pager;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

namespace std {
    template <>
    struct hash<hvox::Chunk> {
        std::size_t operator()(const hvox::Chunk& chunk) const {
            std::hash<hvox::ColumnID> _hash;
            return _hash(chunk.id());
        }
    };
}  // namespace std

#endif  // __hemlock_voxel_chunk_chunk_h