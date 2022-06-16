#ifndef __hemlock_voxel_chunk_h
#define __hemlock_voxel_chunk_h

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 32
#endif

#include "timing.h"
#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/coordinate_system.h"
#include "voxel/chunk/events.hpp"
#include "voxel/chunk/load_task.hpp"
#include "voxel/chunk/state.hpp"

namespace hemlock {
    namespace voxel {
        struct ChunkInstanceData;

        /**
         * @brief 
         */
        struct Chunk : public hmem::Handleable {
            Chunk();

            void init();
            void init(
                                 Block* block_buffer//,
                // ChunkInstanceDataPager& instance_data_pager
            );
            void dispose();

            void update(TimeData);

            ChunkGridPosition position;
            Neighbours        neighbours;
            Block*            blocks;

            RenderState render_state;
            struct {
                ChunkInstanceData*  data;
                ui32                count;
            } instance;

            std::atomic<ChunkState>         state;
            std::atomic<ChunkLoadTaskKind>  pending_task;
            std::atomic<bool>               gen_task_active, mesh_task_active;

            // TODO(Matthew): Remove?
            std::atomic<bool> unload;

            CancellableEvent<BlockChangeEvent>      on_block_change;
            CancellableEvent<BulkBlockChangeEvent>  on_bulk_block_change;

            // NOTE(Matthew): These events, at least on_mesh_change, can be
            //                called from multiple threads. Events are NOT
            //                thread-safe. Our one guarantee is not really
            //                a full guarantee but should hold true: only
            //                one thread that could make a state change
            //                should be processing a task regarding this
            //                chunk at any point in time. If this fails
            //                to hold up, then we could easily get race
            //                conditions inside the events.
            Event<>                                 on_mesh_change;
            Event<RenderState>                      on_render_state_change;
            Event<>                                 on_unload;
        protected:
            void init_events();

            // ChunkInstanceDataPager& m_instance_data_pager;
            bool                    m_owns_blocks;
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
        bool set_block( Chunk* chunk,
            BlockChunkPosition block_position,
                         Block block );
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
        bool set_blocks( Chunk* chunk,
             BlockChunkPosition start_block_position,
             BlockChunkPosition end_block_position,
                          Block block );
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
        bool set_blocks( Chunk* chunk,
             BlockChunkPosition start_block_position,
             BlockChunkPosition end_block_position,
                         Block* blocks );

        template <typename DataType>
        void set_per_block_data( DataType* buffer,
                  hvox::BlockChunkPosition start_block_position,
                  hvox::BlockChunkPosition end_block_position,
                                  DataType data );

        template <typename DataType>
        void set_per_block_data( DataType* buffer,
                  hvox::BlockChunkPosition start_block_position,
                  hvox::BlockChunkPosition end_block_position,
                                 DataType* data );
    }
}
namespace hvox = hemlock::voxel;

namespace std {
    template <>
    struct hash<hvox::Chunk> {
        std::size_t operator()(const hvox::Chunk& chunk) const {
            std::hash<hvox::ColumnID> hash;
            return hash(chunk.position.id);
        }
    };
}

#include "voxel/chunk.inl"

#endif // __hemlock_voxel_chunk_h
