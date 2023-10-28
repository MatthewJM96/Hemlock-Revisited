#ifndef __hemlock_voxel_chunk_events_lod_change_hpp
#define __hemlock_voxel_chunk_events_lod_change_hpp

namespace hemlock {
    namespace voxel {
        using LODLevel = ui8;

        struct LODChangeEvent {
            LODLevel before, after;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_events_lod_change_hpp
