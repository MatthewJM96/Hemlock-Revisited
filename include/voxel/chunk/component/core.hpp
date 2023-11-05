#ifndef __hemlock_voxel_chunk_component_core_hpp
#define __hemlock_voxel_chunk_component_core_hpp

#include "voxel/block_manager.h"
#include "voxel/chunk/state.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        /**
         * @brief Required component for all chunks. Holds minimum state to represent a
         * chunk of voxels in a chunk grid.
         */
        struct ChunkCoreComponent {
            ChunkCoreComponent(
                ChunkGridPosition position_, hmem::Handle<ChunkBlockPager> block_pager
            ) :
                position(position_) {
                blocks.init(block_pager);
            }

            ~ChunkCoreComponent() { blocks.dispose(); }

            ChunkID id() const { return position.id; }

            ChunkGridPosition position;
            Neighbours        neighbours;

            BlockManager blocks;

            // NOTE(Matthew): Chunk events can in-principle be called from
            //                multiple threads. Events are NOT thread-safe.
            //                Our one guarantee is not really a full
            //                guarantee but should hold true: only one
            //                thread that could make a state change should
            //                be processing a task regarding this chunk at
            //                any point in time. If this fails to hold up,
            //                then we could easily get race conditions
            //                inside the events.

            Event<> on_generation;

            CancellableEvent<BlockChangeEvent>     on_before_block_change;
            CancellableEvent<BulkBlockChangeEvent> on_before_bulk_block_change;

            Event<BlockChangeEvent>     on_block_change;
            Event<BulkBlockChangeEvent> on_bulk_block_change;

            // TODO(Matthew): how to do this?
            // void init_events(...);
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

namespace std {
    template <>
    struct hash<hvox::ChunkCoreComponent> {
        std::size_t operator()(const hvox::ChunkCoreComponent& chunk) const {
            std::hash<hvox::ColumnID> _hash;
            return _hash(chunk.id());
        }
    };
}  // namespace std

#endif  // __hemlock_voxel_chunk_component_core_hpp
