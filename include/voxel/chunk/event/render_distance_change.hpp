#ifndef __hemlock_voxel_chunk_event_render_distance_change_hpp
#define __hemlock_voxel_chunk_event_render_distance_change_hpp

namespace hemlock {
    namespace voxel {
        struct RenderDistanceChangeEvent {
            struct {
                ui32 render_distance;
                ui32 chunks_in_render_distance;
            } before, after;
        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_event_render_distance_change_hpp
