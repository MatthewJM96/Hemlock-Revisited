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

// Integral matrix types.
using i8m2 = glm::i8mat2x2;
using i8m3 = glm::i8mat3x3;
using i8m4 = glm::i8mat4x4;

using i16m2 = glm::i16mat2x2;
using i16m3 = glm::i16mat3x3;
using i16m4 = glm::i16mat4x4;

using i32m2 = glm::i32mat2x2;
using i32m3 = glm::i32mat3x3;
using i32m4 = glm::i32mat4x4;

using i64m2 = glm::i64mat2x2;
using i64m3 = glm::i64mat3x3;
using i64m4 = glm::i64mat4x4;

using ui8m2 = glm::u8mat2x2;
using ui8m3 = glm::u8mat3x3;
using ui8m4 = glm::u8mat4x4;

using ui16m2 = glm::u16mat2x2;
using ui16m3 = glm::u16mat3x3;
using ui16m4 = glm::u16mat4x4;

using ui32m2 = glm::u32mat2x2;
using ui32m3 = glm::u32mat3x3;
using ui32m4 = glm::u32mat4x4;

using ui64m2 = glm::u64mat2x2;
using ui64m3 = glm::u64mat3x3;
using ui64m4 = glm::u64mat4x4;

// Floating-point matrix types.
using f32m2 = glm::f32mat2;
using f32m3 = glm::f32mat3;
using f32m4 = glm::f32mat4;

using f64m2 = glm::f64mat2;
using f64m3 = glm::f64mat3;
using f64m4 = glm::f64mat4;

using colour3 = ui8v3;
using colour4 = ui8v4;

namespace hemlock {
    template<typename ReturnType, typename... Args>
    struct Delegate;

    template<typename ReturnType, typename... Args>
    struct Delegate<ReturnType(Args...)> : public std::function<ReturnType(Args...)> { /* Empty. */ };
}

#endif // __hemlock_types_h
