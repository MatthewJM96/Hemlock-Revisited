#ifndef __hemlock_voxel_chunk_h
#define __hemlock_voxel_chunk_h

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

        /**
         * @brief Set the block at the given position in the chunk
         * passed in.
         *
         * @param chunk The chunk in which to set the block.
         * @param block_position The position at which to set the block.
         * @param block The block to set.
         *
         * @return True if the block was set, false otherwise.
         */
        bool set_block(
            hmem::Handle<Chunk> chunk, BlockChunkPosition block_position, Block block
        );
        /**
         * @brief Set all points in a rectangular cuboid of the
         * passed in chunk to a specific block.
         *
         * @param chunk The chunk in which to set the block.
         * @param block_position The position marking the start of
         * the rectangular cuboid to set blocks in.
         * @param block_position The position marking the end of
         * the rectangular cuboid to set blocks in.
         * @param block The block to set.
         *
         * @return True if the blocks were set, false otherwise.
         */
        bool set_blocks(
            hmem::Handle<Chunk> chunk,
            BlockChunkPosition  start_block_position,
            BlockChunkPosition  end_block_position,
            Block               block
        );
        /**
         * @brief Set all points in a rectangular cuboid of the
         * passed in chunk to each block in a buffer. Note, the
         * buffer is assumed to go in x, then y, then z starting
         * from the near bottom left of the chunk.
         *
         * @param chunk The chunk in which to set the block.
         * @param block_position The position at which to set the block.
         * @param block The blocks to set.
         *
         * @return True if the blocks were set, false otherwise.
         */
        bool set_blocks(
            hmem::Handle<Chunk> chunk,
            BlockChunkPosition  start_block_position,
            BlockChunkPosition  end_block_position,
            Block*              blocks
        );

        template <typename DataType>
        void set_per_block_data(
            DataType*                buffer,
            hvox::BlockChunkPosition start_block_position,
            hvox::BlockChunkPosition end_block_position,
            DataType                 data
        );

        template <typename DataType>
        void set_per_block_data(
            DataType*                buffer,
            hvox::BlockChunkPosition start_block_position,
            hvox::BlockChunkPosition end_block_position,
            DataType*                data
        );
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

#include "voxel/chunk/chunk.inl"

#endif  // __hemlock_voxel_chunk_h
