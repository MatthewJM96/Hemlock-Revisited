#include "stdafx.h"

#include "io/image.h"

hio::img::bin::InternalPixelFormat hio::img::bin::convert_pixel_format(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGB_UI8:
            return { 3, 1 };
        case PixelFormat::RGBA_UI8:
            return { 4, 1 };
        case PixelFormat::RGB_UI16:
            return { 3, 2 };
        case PixelFormat::RGBA_UI16:
            return { 4, 2 };
        default:
            // We don't recognise the pixel format.
            return { 0, 0 };
    }
}

bool hio::img::bin::load(std::string filepath, void*& data, ui32v2& dimensions, PixelFormat& format) {
    // Open file, if we can't then fail.
    FILE* file = fopen(filepath.data(), "rb");
    if (file == nullptr) return false;

    /**************************************************\
     * Handle File Header                             *
    \**************************************************/

    BinFileHeader fileHeader;

    // Read in the file header.
    size_t read = fread(&fileHeader, 1, sizeof(BinFileHeader), file);

    // If we didn't manage to read in the entire header's worth of information, fail.
    if (read != sizeof(BinFileHeader)) return false;

    // Fail if file type doesn't match our binary type.
    if (fileHeader.type[0] != BIN_TYPE_1
        || fileHeader.type[1] != BIN_TYPE_2) return false;

    // Fail if the file version doesn't match the version we support.
    if (fileHeader.version != BIN_VERSION) return false;

    /**************************************************\
     * Handle Image Header                            *
    \**************************************************/

    BinImageHeader imgHeader;

    // Read in the image header.
    read = fread(&imgHeader, 1, sizeof(BinImageHeader), file);

    // If we didn't manage to read in the entire header's worth of information, fail.
    if (read != sizeof(BinImageHeader)) return false;

    // If image header size isn't equal to image header struct, leave - our struct may be malformed.
    if (imgHeader.size != sizeof(BinImageHeader)) return false;

    /**************************************************\
     * Extract Image Information                      *
    \**************************************************/

    // If state pixel format is not of a value less than the PixelFormat sentinel, then it is invalid - leave.
    if (imgHeader.pixelFormat >= static_cast<ui32>(PixelFormat::SENTINEL)) return false;

    // Obtain pixel format.
    format = static_cast<PixelFormat>(imgHeader.pixelFormat);

    // Obtain dimensions.
    dimensions.x = imgHeader.width;
    dimensions.y = imgHeader.height;

    // Extract pixel information from format.
    auto [channels, bytesPerChannel] = convert_pixel_format(format);

    // Check the pixel information is valid, if not fail.
    if (channels == 0 || bytesPerChannel == 0) return false;

    // Determine size of needed pixel buffer and create that buffer.
    ui32 imageSize = imgHeader.width * imgHeader.height * channels * bytesPerChannel;
    data = reinterpret_cast<void*>(new ui8[imageSize]);

    // Read pixel data into a buffer.
    read = fread(data, 1, imageSize, file);

    // If we didn't manage to read in the entire pixel data's worth of information, fail.
    if (read != imageSize) return false;

    // Close file and return.
    fclose(file);

    return true;
}

bool hio::img::bin::save(std::string filepath, const void* data, ui32v2 dimensions, PixelFormat format) {
    // Extract pixel information from format.
    auto [channels, bytesPerChannel] = convert_pixel_format(format);

    // Determine image size in bytes.
    ui32 imageSize = channels * bytesPerChannel * dimensions.x * dimensions.y;

    // Set up the image header.
    BinImageHeader imgHeader;

    imgHeader.size        = sizeof(BinImageHeader);
    imgHeader.width       = dimensions.x;
    imgHeader.height      = dimensions.y;
    imgHeader.imageSize   = imageSize;
    imgHeader.pixelFormat = static_cast<ui8>(format);
    imgHeader.compression = 0;
    imgHeader.reserved    = 0;

    // Set up the file header.
    BinFileHeader fileHeader;

    fileHeader.type[0]  = BIN_TYPE_1;
    fileHeader.type[1]  = BIN_TYPE_2;
    fileHeader.version  = BIN_VERSION;
    fileHeader.size     = sizeof(BinFileHeader) + sizeof(BinImageHeader) + imgHeader.imageSize;
    fileHeader.offset   = sizeof(BinFileHeader) + sizeof(BinImageHeader);
    fileHeader.reserved = 0;

    // Open the file desired, and if we couldn't, fail.
    FILE* file = fopen(filepath.data(), "wb");
    if (file == nullptr) return false;

    // Write the file header, if we couldn't, fail.
    size_t written = fwrite(&fileHeader, 1, sizeof(BinFileHeader), file);
    if (written != sizeof(BinFileHeader)) return false;

    // Write the image header, if we couldn't, fail.
    written = fwrite(&imgHeader, 1, sizeof(BinImageHeader), file);
    if (written != sizeof(BinImageHeader)) return false;

    // Write the data, if we couldn't, fail.
    written = fwrite(data, 1, imageSize, file);
    if (written != imageSize) return false;

    // Close the file and return.
    fclose(file);

    return true;
}

hio::img::png::InternalPixelFormat hio::img::png::convert_pixel_format(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGB_UI8:
            return { PNG_COLOR_TYPE_RGB, 8 };
        case PixelFormat::RGB_UI16:
            return { PNG_COLOR_TYPE_RGB, 16 };
        case PixelFormat::RGBA_UI8:
            return { PNG_COLOR_TYPE_RGB_ALPHA, 8 };
        case PixelFormat::RGBA_UI16:
            return { PNG_COLOR_TYPE_RGB_ALPHA, 16 };
        default:
            return { PNG_COLOR_TYPE_GRAY, 0 };
    }
}

hio::img::PixelFormat hio::img::png::convert_internal_pixel_format(InternalPixelFormat format) {
    switch (format.first) {
        case PNG_COLOR_TYPE_RGB:
            switch (format.second) {
                case 8:
                    return PixelFormat::RGB_UI8;
                case 16:
                    return PixelFormat::RGB_UI16;
            }
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            switch (format.second) {
                case 8:
                    return PixelFormat::RGBA_UI8;
                case 16:
                    return PixelFormat::RGBA_UI16;
            }
            break;
    }
    return PixelFormat::SENTINEL;
}

bool hio::img::png::load(std::string filepath, void*& data, ui32v2& dimensions, PixelFormat& format) {
    // Open the image file we will save to.
    FILE* file = fopen(filepath.data(), "rb");
    // Check we successfully opened the file.
    if (!file) return false;

    // Read header and compare for PNG signature.
    ui8 header[8];
    fread(header, 1, 8, file);
    if (png_sig_cmp(header, 0, 8)) return false;

    // Set up handler that will be used to read the data.
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    // Check the handler was set up correctly, if not close file and return.
    if (!png) {
        fclose(file);
        return false;
    }

    // This will contain the header information about the image (things like width, height, compression type).
    png_infop info = png_create_info_struct(png);
    // Check we got a valid info struct, if not close file, clean up handler and return.
    if (!info) {
        fclose(file);
        // Note that png_infopp_NULL is because the info struct is null!
        png_destroy_write_struct(&png, nullptr);
        return false;
    }

    // Set up an error handler for PNG reading. If that fails, close file, clean up handler and return.
    if (setjmp(png_jmpbuf(png))) {
        fclose(file);
        png_destroy_write_struct(&png, &info);
        return false;
    }

    // Pass the file to the PNG handler.
    png_init_io(png, file);
    // Set number of signature bytes.
    png_set_sig_bytes(png, 8);

    // Read properties of the image to be loaded.
    png_read_info(png, info);

    // Write out dimensions of the image.
    dimensions = ui32v2{
        png_get_image_width(png, info),
        png_get_image_height(png, info)
    };

    // Write out pixel format of the image.
    format = convert_internal_pixel_format({
        png_get_color_type(png, info),
        png_get_bit_depth(png, info)
    });

    // Unsure how useful these two lines are.
    // Presumably sets interlacing state, but given
    // we're reading how would we know more than
    // libpng? We're not passing anything in at any
    // rate...
    png_set_interlace_handling(png);
    png_read_update_info(png, info);

    // Get number of bytes per row of the image.
    ui32 row_size = png_get_rowbytes(png, info);

    // Allocate the buffer we'll be reading into.
    data = reinterpret_cast<void*>(new ui8[dimensions.y * row_size]);

    png_bytep* rows = new png_bytep[dimensions.y];
    for (ui32 y = 0; y < dimensions.y; ++y) {
        ui32 row_idx = y * row_size;
        rows[y] = &reinterpret_cast<png_bytep>(data)[row_idx];
    }

    // Read the PNG.
    png_read_image(png, rows);

    // Clean up the handler.
    png_destroy_read_struct(&png, &info, nullptr);

    // Close file.
    fclose(file);

    // Clean up rows array.
    delete[] rows;

    return true;
}

bool hio::img::png::save(std::string filepath, const void* data, ui32v2 dimensions, PixelFormat format) {
    // Open the image file we will save to.
    FILE* file = fopen(filepath.data(), "wb");
    // Check we successfully opened the file.
    if (!file) return false;

    // Set up handler that will be used to write the data.
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    // Check the handler was set up correctly, if not close file and return.
    if (!png) {
        fclose(file);
        return false;
    }

    // This will contain the header information about the image (things like width, height, compression type).
    png_infop info = png_create_info_struct(png);
    // Check we got a valid info struct, if not close file, clean up handler and return.
    if (!info) {
        fclose(file);
        // Note that png_infopp_NULL is because the info struct is null!
        png_destroy_write_struct(&png, nullptr);
        return false;
    }

    // Set up an error handler for PNG reading. If that fails, close file, clean up handler and return.
    if (setjmp(png_jmpbuf(png))) {
        fclose(file);
        png_destroy_write_struct(&png, &info);
        return false;
    }

    // Pass the file to the PNG handler.
    png_init_io(png, file);

    // Get the PNG properties of the chosen pixel format.
    auto [colour_type, bit_depth] = convert_pixel_format(format);

    // Set the PNG properties we want.
    png_set_IHDR(
        png,
        info,
        dimensions.x,
        dimensions.y,
        bit_depth,
        colour_type,
        // Note these next three are set to the most default possible, we don't really care about them.
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    // Write those properties to the file.
    png_write_info(png, info);

    // Determine the depth of the image in bytes.
    ui8 depth = bit_depth / 8;

    // Determine the number of colour channels we have (e.g. RGB has 3, one for red, for green and for blue).
    ui8 channels = 0;
    switch (colour_type) {
        case PNG_COLOR_TYPE_GRAY:
            channels = 1;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            channels = 2;
            break;
        case PNG_COLOR_TYPE_RGB:
            channels = 3;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            channels = 4;
            break;
    }

    // Begin preparing data for writing.
    png_bytep* rows  = new png_bytep[dimensions.y];
    png_byte*  image = static_cast<png_byte*>(const_cast<void*>(data));

    // The position were at in the data.
    size_t pos    = 0;
    // The amount position should be incremented by per pixel row processed.
    //     This is determined by considering that we have channels many bits of data per pixel,
    //     and each channel is depth bytes large. The number of bytes wide a row is is then just
    //     the number of pixels in a row multiplied by these two numbers.
    size_t stride = dimensions.x * channels * depth;

    // Iterate over each row, setting the corresponding row in our array "rows" to point
    // to the start of that row inside the image data "image".
    for (size_t row = 0; row < dimensions.y; ++row) {
        rows[row] = &image[pos];

        pos += stride;
    }

    // Write the image!
    png_write_image(png, rows);
    png_write_end(png, info);

    // Clean up the handler.
    png_destroy_write_struct(&png, &info);

    // Close file.
    fclose(file);

    // Clean up rows array.
    delete[] rows;

    return true;
}
