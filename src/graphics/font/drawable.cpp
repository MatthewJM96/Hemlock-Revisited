#include "stdafx.h"

#include "graphics/font/drawable.h"

H_DEF_UNION_WITH_SERIALISATION(
    hemlock::graphics::font,
    StringSizing,
    ui8,
    (SCALED, H_NON_POD_TYPE(), (scaling, f32v2)),
    (FIXED, H_POD_STRUCT(), (scale_x, f32), (target_height, f32)),
)
