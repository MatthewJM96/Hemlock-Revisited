#ifndef __hemlock_voxel_state_h
#define __hemlock_voxel_state_h

namespace hemlock {
    namespace voxel {
        enum class Voxel : ui64 {
        };

        const Voxel NULL_VOXEL{};
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

namespace std {
    ostream& operator<<(ostream& os, const hvox::Voxel& voxel);

    istream& operator<<(istream& is, hvox::Voxel& voxel);
}  // namespace std

#endif  // __hemlock_voxel_state_h
