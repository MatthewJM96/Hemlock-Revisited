#ifndef __hemlock_voxel_face_check_hpp
#define __hemlock_voxel_face_check_hpp

#include "voxel/chunk/constants.hpp"

namespace hemlock {
    namespace voxel {
        static inline bool is_at_left_face(hvox::VoxelIndex index) {
            return (index % CHUNK_LENGTH) == 0;
        }

        static inline bool is_at_right_face(hvox::VoxelIndex index) {
            return ((index + 1) % CHUNK_LENGTH) == 0;
        }

        static inline bool is_at_bottom_face(hvox::VoxelIndex index) {
            return (index % (CHUNK_AREA)) < CHUNK_LENGTH;
        }

        static inline bool is_at_top_face(hvox::VoxelIndex index) {
            return (index % (CHUNK_AREA)) >= (CHUNK_LENGTH * (CHUNK_LENGTH - 1));
        }

        static inline bool is_at_front_face(hvox::VoxelIndex index) {
            return index < (CHUNK_AREA);
        }

        static inline bool is_at_back_face(hvox::VoxelIndex index) {
            return index >= (CHUNK_AREA * (CHUNK_LENGTH - 1));
        }

        static inline hvox::VoxelIndex index_at_right_face(hvox::VoxelIndex index) {
            return index + CHUNK_LENGTH - 1;
        }

        static inline hvox::VoxelIndex index_at_left_face(hvox::VoxelIndex index) {
            return index - CHUNK_LENGTH + 1;
        }

        static inline hvox::VoxelIndex index_at_top_face(hvox::VoxelIndex index) {
            return index + (CHUNK_LENGTH * (CHUNK_LENGTH - 1));
        }

        static inline hvox::VoxelIndex index_at_bottom_face(hvox::VoxelIndex index) {
            return index - (CHUNK_LENGTH * (CHUNK_LENGTH - 1));
        }

        static inline hvox::VoxelIndex index_at_front_face(hvox::VoxelIndex index) {
            return index - (CHUNK_AREA * (CHUNK_LENGTH - 1));
        }

        static inline hvox::VoxelIndex index_at_back_face(hvox::VoxelIndex index) {
            return index + (CHUNK_AREA * (CHUNK_LENGTH - 1));
        }
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_face_check_hpp
