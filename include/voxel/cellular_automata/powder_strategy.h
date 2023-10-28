#ifndef __hemlock_voxel_cellular_automata_powder_strategy_h
#define __hemlock_voxel_cellular_automata_powder_strategy_h

namespace hemlock {
    namespace voxel {
        class Chunk;
        class ChunkGrid;

        struct CAPowderStrategy {
            bool can_run(hmem::Handle<ChunkGrid> chunk_grid, hmem::Handle<Chunk> chunk)
                const;

            void operator()(
                hmem::Handle<ChunkGrid> chunk_grid, hmem::Handle<Chunk> chunk
            ) const;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_cellular_automata_powder_strategy_h
