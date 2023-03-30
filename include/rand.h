#ifndef __hemlock_rand_h
#define __hemlock_rand_h

namespace hemlock {
    template <std::floating_point Precision>
    Precision global_unitary_rand();

    template <std::floating_point Precision>
    Precision global_rand(Precision min, Precision max);
}  // namespace hemlock

#include "rand.inl"

#endif  // __hemlock_rand_h
