#ifndef __hemlock_tests_ai_screen_dimension_hpp
#define __hemlock_tests_ai_screen_dimension_hpp

namespace dimension {
    struct Map2DDimensions {
        size_t x, y;
    };

    constexpr size_t dim_to_padded_dim(size_t dim) {
        return dim + 2;
    }

    constexpr size_t dim2d_to_padded_size(size_t dim) {
        return dim_to_padded_dim(dim) * dim_to_padded_dim(dim);
    }

    constexpr size_t dim2d_to_padded_size(size_t dim_x, size_t dim_y) {
        return dim_to_padded_dim(dim_x) * dim_to_padded_dim(dim_y);
    }

    constexpr size_t dim3d_to_padded_size(size_t dim) {
        return dim_to_padded_dim(dim) * dim_to_padded_dim(dim) * dim_to_padded_dim(dim);
    }

    constexpr size_t dim3d_to_padded_size(size_t dim_x, size_t dim_y, size_t dim_z) {
        return dim_to_padded_dim(dim_x) * dim_to_padded_dim(dim_y)
               * dim_to_padded_dim(dim_z);
    }
};  // namespace dimension

#endif  // __hemlock_tests_ai_screen_dimension_hpp
