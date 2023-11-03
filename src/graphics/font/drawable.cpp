#include "stdafx.h"

#include "graphics/font/drawable.h"

H_DEF_UNION_WITH_SERIALISATION(
    hemlock::graphics::font,
    StringSizing,
    ui8,
    (H_NON_POD_TYPE(), SCALED, (scaling, f32v2)),
    (H_POD_STRUCT(), FIXED, (scale_x, f32), (target_height, f32)),
)
