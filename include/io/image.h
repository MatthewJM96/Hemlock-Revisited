#ifndef __hemlock_io_image_h
#define __hemlock_io_image_h

namespace hemlock {
    namespace io {
        namespace image {
            /**
             * @brief Formats that pixels can be stored in.
             */
            enum class PixelFormat : ui8 {
                RGB_UI8 = 0,
                RGBA_UI8,
                RGB_UI16,
                RGBA_UI16,
                SENTINEL
            };

            using Loader = std::function<bool(const char*, void*&, ui32v2&, PixelFormat&)>;
            using Saver  = std::function<bool(const char*, const void*, ui32v2, PixelFormat)>;

            namespace binary {
                const ui8  BIN_TYPE_1  = 'S';
                const ui8  BIN_TYPE_2  = 'P';
                const ui32 BIN_VERSION = 1;

                /**
                 * @brief The standardised file header for binary storage.
                 */
                struct BinFileHeader {
                    ui8  type[2]; // The file type - ALWAYS set first byte to 'S' and second to 'P'.
                    ui32 version; // The version of the binary file type used.
                    ui32 size;    // The size of the file in bytes including headers.
                    ui32 offset;  // Offset in bytes to image data (i.e. sum of sizes of the headers).
                    ui16 reserved;
                };

                /**
                 * @brief The standardised image header for binary storage.
                 */
                struct BinImageHeader {
                    ui32 size;        // The size of this header in bytes.
                    ui32 width;       // The width of the image in pixels.
                    ui32 height;      // The height of the image in pixels.
                    ui32 imageSize;   // The image size in bytes (unused for uncompressed - any value for such images is allowed).
                    ui8  pixelFormat; // The pixel format of the image.
                    ui8  compression; // The compression used (0 for uncompressed).
                    ui16 reserved;
                };

                using InternalPixelFormat = std::pair<ui32, ui32>;

                /**
                 * @brief Converts an general PixelFormat value to the
                 * corresponding binary properties.
                 *
                 * @param format The pixel format to convert.
                 *
                 * @return The determined binary properties.
                 */
                InternalPixelFormat convert_pixel_format(PixelFormat format);

                /**
                 * @brief Loads an image from the named binary file.
                 * 
                 * @param filepath The filepath to locate an image at.
                 * @param data The buffer into which the image is stored.
                 * @param dimensions The discovered dimensions of the image.
                 * @param format The discovered pixel format of the image.
                 * @return true when the image is successfully loaded, false otherwise.
                 */
                bool load(const char* filepath, CALLER_DELETE OUT void*& data, OUT ui32v2& dimensions, OUT PixelFormat& format);

                /**
                 * @brief Saves an image as a binary.file.
                 * 
                 * @param filepath The file to save the image to.
                 * @param data The buffer containing the image to save.
                 * @param dimensions The dimensions of the image to save.
                 * @param format The pixel format of the image to save.
                 * @return true when the image is successfully saved, false otherwise.
                 */
                bool save(const char* filepath, IN const void* data, ui32v2 dimensions, PixelFormat format);
            }

            namespace png {
                using InternalPixelFormat = std::pair<png_byte, png_byte>;

                /**
                 * @brief Converts an general PixelFormat value to the
                 * corresponding PNG properties.
                 *
                 * @param format The pixel format to convert.
                 *
                 * @return The determined PNG properties.
                 */
                InternalPixelFormat convert_pixel_format(PixelFormat format);

                /**
                 * @brief Loads an image from the named PNG file.
                 * 
                 * @param filepath The filepath to locate an image at.
                 * @param data The buffer into which the image is stored.
                 * @param dimensions The discovered dimensions of the image.
                 * @param format The discovered pixel format of the image.
                 * @return true when the image is successfully loaded, false otherwise.
                 */
                bool load(const char* filepath, CALLER_DELETE OUT  void*& data, ui32v2& dimensions, PixelFormat& format);

                /**
                 * @brief Saves an image as a PNG.file.
                 * 
                 * @param filepath The file to save the image to.
                 * @param data The buffer containing the image to save.
                 * @param dimensions The dimensions of the image to save.
                 * @param format The pixel format of the image to save.
                 * @return true when the image is successfully saved, false otherwise.
                 */
                bool save(const char* filepath, IN const void* data, ui32v2 dimensions, PixelFormat format);
            }
        }
    }
}
namespace hio         = hemlock::io;
namespace hio_img     = hio::image;
namespace hio_img_bin = hio_img::binary;
namespace hio_img_png = hio_img::png;

#endif // __hemlock_io_image_h
