#ifndef __hemlock_tests_ai_screen_dimension_hpp
#define __hemlock_tests_ai_screen_dimension_hpp

namespace dimension {
    struct Map2DDimensions {
        ui32 x, y;
    };

    constexpr ui32 dim_to_padded_dim(ui32 dim) {
        return dim + 2;
    }

    constexpr ui32 dim2d_to_padded_size(ui32 dim) {
        return dim_to_padded_dim(dim) * dim_to_padded_dim(dim);
    }

    constexpr ui32 dim2d_to_padded_size(ui32 dim_x, ui32 dim_y) {
        return dim_to_padded_dim(dim_x) * dim_to_padded_dim(dim_y);
    }

    constexpr ui32 dim3d_to_padded_size(ui32 dim) {
        return dim_to_padded_dim(dim) * dim_to_padded_dim(dim) * dim_to_padded_dim(dim);
    }

    constexpr ui32 dim3d_to_padded_size(ui32 dim_x, ui32 dim_y, ui32 dim_z) {
        return dim_to_padded_dim(dim_x) * dim_to_padded_dim(dim_y)
               * dim_to_padded_dim(dim_z);
    }
};  // namespace dimension

#endif  // __hemlock_tests_ai_screen_dimension_hpp
