#include "stdafx.h"

#include "debug/heatmap.h"

#include "io/image.h"

void hdeb::print_heatmaps(
    heatmap_t*         heatmaps,
    size_t             heatmap_count,
    const std::string& output_dir,
    const std::string& tag
) {
    for (size_t heatmap_idx = 0; heatmap_idx < heatmap_count; ++heatmap_idx) {
        print_heatmap(&heatmaps[heatmap_idx], heatmap_idx, output_dir, tag);
    }
}

void hdeb::print_heatmap(
    heatmap_t*         heatmap,
    size_t             idx,
    const std::string& output_dir,
    const std::string& tag
) {
    ui8* image_buffer = new ui8[heatmap->w * heatmap->h * 4];

    heatmap_render_to(heatmap, heatmap_cs_default, image_buffer);

    std::string numeric_code = std::to_string(idx);
    if (numeric_code.size() < 10) numeric_code.insert(0, 10 - numeric_code.size(), '0');

    hio::img::png::save(
        output_dir + "/" + tag + "." + numeric_code + ".png",
        image_buffer,
        { heatmap->w, heatmap->h },
        hio::img::PixelFormat::RGBA_UI8
    );

    delete[] image_buffer;
}
