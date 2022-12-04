#ifndef __hemlock_debug_heatmap_hpp
#define __hemlock_debug_heatmap_hpp

#include <libheatmap/heatmap.h>

namespace hemlock {
    namespace debug {
        static float SQUARE_STAMP_DATA[]
            = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

        static heatmap_stamp_t SQUARE_STAMP [[maybe_unused]]
        = { SQUARE_STAMP_DATA, 9, 9 };

        void print_heatmaps(
            heatmap_t*         heatmaps,
            size_t             heatmap_count,
            const std::string& output_dir,
            const std::string& tag
        );
        void print_heatmap(
            heatmap_t*         heatmap,
            size_t             idx,
            const std::string& output_dir,
            const std::string& tag
        );
    }  // namespace debug
}  // namespace hemlock
namespace hdeb = hemlock::debug;

#endif  // __hemlock_debug_heatmap_hpp
