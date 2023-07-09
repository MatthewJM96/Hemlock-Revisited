#include "stdafx.h"

#include "graphics/font/text_align.h"

H_DEF_ENUM_WITH_SERIALISATION(hemlock::graphics::font, TextAlign)

f32v2 hg::f::calculate_align_offset(
    TextAlign align, f32v4 rect, f32 height, f32 length
) {
    switch (align) {
        case TextAlign::NONE:
        case TextAlign::TOP_LEFT:
            return f32v2(0.0f, 0.0f);
        case TextAlign::CENTER_LEFT:
            return f32v2(0.0f, (rect.w - height) / 2.0f);
        case TextAlign::BOTTOM_LEFT:
            return f32v2(0.0f, rect.w - height);
        case TextAlign::TOP_CENTER:
            return f32v2((rect.z - length) / 2.0f, 0.0f);
        case TextAlign::CENTER_CENTER:
            return f32v2((rect.z - length) / 2.0f, (rect.w - height) / 2.0f);
        case TextAlign::BOTTOM_CENTER:
            return f32v2((rect.z - length) / 2.0f, rect.w - height);
            break;
        case TextAlign::TOP_RIGHT:
            return f32v2(rect.z - length, 0.0f);
        case TextAlign::CENTER_RIGHT:
            return f32v2(rect.z - length, (rect.w - height) / 2.0f);
        case TextAlign::BOTTOM_RIGHT:
            return f32v2(rect.z - length, rect.w - height);
        default:
            // Should never get here.
            assert(false);
    }
    // Really should never get here.
    return {};
}
