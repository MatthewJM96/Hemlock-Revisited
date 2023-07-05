#include "stdafx.h"

#include "graphics/sprite/sprite.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::graphics::sprite,
    SpriteData,
    (texture, GLuint),
    (position, f32v2),
    (size, f32v2),
    (depth, f32),
    (uv_rect, f32v4),
    (c1, colour4),
    (c2, colour4),
    (gradient, hemlock::graphics::Gradient)
    // TODO(Matthew): Offsets, rotations, etc?
    // TODO(Matthew): Custom gradients? Different blending styles?
)
