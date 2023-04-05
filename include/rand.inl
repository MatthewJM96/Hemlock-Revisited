#include "stdafx.h"

#include "rand.h"

template <std::floating_point Precision>
Precision hemlock::global_unitary_rand() {
#if defined(DEBUG)
    static std::mt19937 generator(1337);
#else
    static std::random_device rand_dev;
    static std::mt19937       generator(rand_dev());
#endif  // defined(DEBUG)
    static std::uniform_real_distribution<f32> distribution(0.0f, 1.0f);

    return distribution(generator);
}

template <std::floating_point Precision>
Precision hemlock::global_rand(Precision min, Precision max) {
#if defined(DEBUG)
    static std::mt19937 generator(1337);
#else
    static std::random_device rand_dev;
    static std::mt19937       generator(rand_dev());
#endif  // defined(DEBUG)

    std::uniform_real_distribution<Precision> distribution(min, max);

    return distribution(generator);
}
