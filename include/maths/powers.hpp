#ifndef __hemlock_maths_powers_h
#define __hemlock_maths_powers_h

namespace hemlock {
    namespace maths {
        /**
         * @brief Determines the next power of 2 after the given value and returns it.
         *
         * @param The value to determine the following power of 2 for.
         *
         * @return The power of 2 determined.
         */
        constexpr ui8  next_power_2(ui8 value);
        constexpr ui16 next_power_2(ui16 value);
        constexpr ui32 next_power_2(ui32 value);
        constexpr ui64 next_power_2(ui64 value);
    }  // namespace maths
}  // namespace hemlock
namespace hmaths = hemlock::maths;

#include "powers.inl"

#endif  // __hemlock_maths_powers_h
