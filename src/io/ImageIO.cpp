#include "stdafx.h"
#include "io/ImageIO.h"

#include <png.h>

/**
 * @brief Converts an spio::Image::PixelFormat value to the corresponding PNG properties.
 *
 * @param format The format to convert.
 *
 * @return The determined PNG properties.
 */
std::pair<png_byte, png_byte> convertPixelFormat(spio::Image::PixelFormat format) {
    using spio::Image::PixelFormat;
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

bool spio::Image::save(const char* filepath, const void* data, ui32v2 dimensions, PixelFormat format) {
    // Open the image file we will save to.
    FILE* file = fopen(filepath, "wb");
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
    auto [colourType, bitDepth] = convertPixelFormat(format);

    // Set the PNG properties we want.
    png_set_IHDR(
        png,
        info,
        dimensions.x,
        dimensions.y,
        bitDepth,
        colourType,
        // Note these next three are set to the most default possible, we don't really care about them.
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    // Write those properties to the file.
    png_write_info(png, info);

    // Determine the depth of the image in bytes.
    ui8 depth = bitDepth / 8;

    // Determine the number of colour channels we have (e.g. RGB has 3, one for red, for green and for blue).
    ui8 channels = 0;
    switch (colourType) {
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

    return true;
}
