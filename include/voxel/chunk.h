#ifndef __hemlock_voxel_chunk_h
#define __hemlock_voxel_chunk_h

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 32
#endif

#include "timing.h"
#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Chunk;

        enum class ChunkState {
            NONE,
            GENERATED,
            MESHED,
            MESH_UPLOADED
        };

        enum class ChunkGenKind {
            NONE,
            GENERATE,
            MESH
        };

        struct ChunkGenTaskContext {
            volatile bool stop;
        };
        using ChunkGenThreadState = Thread<ChunkGenTaskContext>::State;
        using ChunkGenTaskQueue   = TaskQueue<ChunkGenTaskContext>;
        class ChunkGenTask : public IThreadTask<ChunkGenTaskContext> {
        public:
            void init(Chunk* chunk) { m_chunk = chunk; }
        protected:
            Chunk* m_chunk;
        };

        struct BlockChangeEvent {
            Chunk*              chunk;
            Block               old_block;
            Block               new_block;
            BlockChunkPosition  block_position;
        };

        struct BulkBlockChangeEvent {
            Chunk*              chunk;
            Block*              new_blocks;
            bool                single_block;
            BlockChunkPosition  start_position;
            BlockChunkPosition  end_position;
        };

        struct Neighbours {
            Chunk *left, *right, *top, *bottom, *front, *back;
        };
        const Neighbours NULL_NEIGHBOURS = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

        /**
         * @brief 
         */
        struct Chunk {
            Chunk();

            void init();
            void init(Block* _blocks);
            void dispose();

            void update(TimeData);

            Neighbours neighbours;

            Block* blocks;

            ChunkGridPosition position;

            ChunkState state;

            hg::MeshData3D_32 mesh;
            hg::MeshHandles mesh_handles;

            struct {
                bool  gen_task_active : 4;
                bool mesh_task_active : 4;
            };

            // TODO(Matthew): Store these here? Somehow feels dodgy.
            CancellableEvent<BlockChangeEvent>     on_block_change;
            CancellableEvent<BulkBlockChangeEvent> on_bulk_block_change;
        protected:
            bool m_owns_blocks;
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
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_h
