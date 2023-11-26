// This is a rather lovely bit manipulation function.
// Essentially, all we're doing in this is up until the return
// statement we take a value like 0110110000110101101 and change
// it to become 0111111111111111111 so that when we add 1 (i.e.
// 0000000000000000001) it becomes 1000000000000000000 -> the
// next power of 2! For exact powers of 2, the initial subtraction
// ensures they are returned directly.

constexpr ui8 hmaths::next_power_2(ui8 value) {
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    return ++value;
}

constexpr ui16 hmaths::next_power_2(ui16 value) {
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    return ++value;
}

constexpr ui32 hmaths::next_power_2(ui32 value) {
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return ++value;
}

constexpr ui64 hmaths::next_power_2(ui64 value) {
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return ++value;
}

#if defined(HEMLOCK_OS_MAC)
// Note that size_t is uniquely defined independent of uint64_t on Mac.
constexpr size_t hmaths::next_power_2(size_t value) {
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return ++value;
}
#endif  // defined(HEMLOCK_OS_MAC)
