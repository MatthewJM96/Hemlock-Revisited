#ifndef __hemlock_types_h
#define __hemlock_types_h

#include <cstdint>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Integral types.
using   i8 = int8_t; ///< 8-bit signed integer
using  i16 = int16_t; ///< 16-bit signed integer
using  i32 = int32_t; ///< 32-bit signed integer
using  i64 = int64_t; ///< 64-bit signed integer
using  ui8 = uint8_t; ///< 8-bit unsigned integer
using ui16 = uint16_t; ///< 16-bit unsigned integer
using ui32 = uint32_t; ///< 32-bit unsigned integer
using ui64 = uint64_t; ///< 64-bit unsigned integer

// Floating-point types.
using f32 = float; ///< 32-bit floating point value (single)
using f64 = double; ///< 64-bit floating point value (double)

// Integral vector types.
using   i8v2 = glm::i8vec2;
using   i8v3 = glm::i8vec3;
using   i8v4 = glm::i8vec4;

using  i16v2 = glm::i16vec2;
using  i16v3 = glm::i16vec3;
using  i16v4 = glm::i16vec4;

using  i32v2 = glm::i32vec2;
using  i32v3 = glm::i32vec3;
using  i32v4 = glm::i32vec4;

using  i64v2 = glm::i64vec2;
using  i64v3 = glm::i64vec3;
using  i64v4 = glm::i64vec4;

using  ui8v2 = glm::u8vec2;
using  ui8v3 = glm::u8vec3;
using  ui8v4 = glm::u8vec4;

using ui16v2 = glm::u16vec2;
using ui16v3 = glm::u16vec3;
using ui16v4 = glm::u16vec4;

using ui32v2 = glm::u32vec2;
using ui32v3 = glm::u32vec3;
using ui32v4 = glm::u32vec4;

using ui64v2 = glm::u64vec2;
using ui64v3 = glm::u64vec3;
using ui64v4 = glm::u64vec4;

// Floating-point vector types.
using f32v2 = glm::f32vec2;
using f32v3 = glm::f32vec3;
using f32v4 = glm::f32vec4;

using f64v2 = glm::f64vec2;
using f64v3 = glm::f64vec3;
using f64v4 = glm::f64vec4;

// Floating-point quaternion types.
using f32q = glm::quat;
using f64q = glm::dquat;

// Floating-point matrix types.
using f32m2 = glm::f32mat2;
using f32m3 = glm::f32mat3;
using f32m4 = glm::f32mat4;

using f64m2 = glm::f64mat2;
using f64m3 = glm::f64mat3;
using f64m4 = glm::f64mat4;

// Colours
struct colour3 {
    union {
        struct {
            ui8 r, g, b;
        };
        ui8 data[3];
    };

    colour3() :
        r(0), g(0), b(0)
    { /* Empty */ }
    constexpr colour3(ui8 r_, ui8 g_, ui8 b_) :
        r(r_), g(g_), b(b_)
    { /* Empty */ }

    bool operator==(const colour3& rhs) const {
        return r == rhs.r
            && g == rhs.g
            && b == rhs.b;
    }

    const ui8& operator[] (const size_t& i) const {
        return data[i];
    }
    ui8& operator[] (const size_t& i) {
        return data[i];
    }
};
struct colour4 {
    union {
        struct {
            ui8 r, g, b, a;
        };
        ui8 data[4];
    };

    colour4() :
        r(0), g(0), b(0), a(0)
    { /* Empty */ }
    constexpr colour4(ui8 r_, ui8 g_, ui8 b_, ui8 a_) :
        r(r_), g(g_), b(b_), a(a_)
    { /* Empty */ }

    bool operator==(const colour4& rhs) const {
        return r == rhs.r
            && g == rhs.g
            && b == rhs.b
            && a == rhs.a;
    }

    const ui8& operator[] (const size_t& i) const {
        return data[i];
    }
    ui8& operator[] (const size_t& i) {
        return data[i];
    }
};

struct TimeData {
    f64 total;
    f64 frame;
};

namespace hemlock {
    template<typename ReturnType, typename... Args>
    using Delegate = std::function<ReturnType(Args...)>;
}

#endif // __hemlock_types_h
