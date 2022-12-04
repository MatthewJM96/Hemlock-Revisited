#ifndef __hemlock_graphics_gradient_hpp
#define __hemlock_graphics_gradient_hpp

namespace hemlock {
    namespace graphics {
        /**
         * @brief Provides an enumeration of the possible gradient styles.
         */
        enum class Gradient {
            NONE,
            LEFT_TO_RIGHT,
            TOP_TO_BOTTOM,
            TOP_LEFT_TO_BOTTOM_RIGHT,
            TOP_RIGHT_TO_BOTTOM_LEFT
        };

        /**
         * @brief Provides linear interpolation between two colours to a given mix.
         *
         * @tparam Precision The type to use for the ratio for interpolating - this
         * must be a floating-point type.
         *
         * @param c1 The first colour to use for the lerp.
         * @param c2 The second colour to use for the lerp.
         * @param ratio The ratio in which to mix the two colours.
         *
         * @return The mix of the two colours determined. At a ratio of 0 the first
         * colour is returned and at a ratio of 1 the second colour is returned.
         * Values between 0 and 1 return a mix between the two colours in a linear
         * fashion.
         */
        template <std::floating_point Precision = f32>
        colour3 lerp(colour3 c1, colour3 c2, Precision ratio) {
            return { static_cast<ui8>(
                         static_cast<Precision>(c1.r) * (1.0 - ratio)
                         + static_cast<Precision>(c2.r) * ratio
                     ),
                     static_cast<ui8>(
                         static_cast<Precision>(c1.g) * (1.0 - ratio)
                         + static_cast<Precision>(c2.g) * ratio
                     ),
                     static_cast<ui8>(
                         static_cast<Precision>(c1.b) * (1.0 - ratio)
                         + static_cast<Precision>(c2.b) * ratio
                     ) };
        }

        /**
         * @brief Provides linear interpolation between two colours to a given mix.
         *
         * @tparam Precision The type to use for the ratio for interpolating - this
         * must be a floating-point type.
         *
         * @param c1 The first colour to use for the lerp.
         * @param c2 The second colour to use for the lerp.
         * @param ratio The ratio in which to mix the two colours.
         *
         * @return The mix of the two colours determined. At a ratio of 0 the first
         * colour is returned and at a ratio of 1 the second colour is returned.
         * Values between 0 and 1 return a mix between the two colours in a linear
         * fashion.
         */
        template <std::floating_point Precision = f32>
        colour4 lerp(colour4 c1, colour4 c2, Precision ratio) {
            return { static_cast<ui8>(
                         static_cast<Precision>(c1.r) * (1.0 - ratio)
                         + static_cast<Precision>(c2.r) * ratio
                     ),
                     static_cast<ui8>(
                         static_cast<Precision>(c1.g) * (1.0 - ratio)
                         + static_cast<Precision>(c2.g) * ratio
                     ),
                     static_cast<ui8>(
                         static_cast<Precision>(c1.b) * (1.0 - ratio)
                         + static_cast<Precision>(c2.b) * ratio
                     ),
                     static_cast<ui8>(
                         static_cast<Precision>(c1.a) * (1.0 - ratio)
                         + static_cast<Precision>(c2.a) * ratio
                     ) };
        }
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_gradient_hpp
