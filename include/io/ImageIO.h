/**
 * @file ImageIO.h
 * @brief Provides helper functions to load and save images.
 */

#pragma once

#if !defined(SP_IO_ImageIO_h__)
#define SP_IO_ImageIO_h__

#include "types.h"

namespace SecretProject {
    namespace io {
        namespace Image {
            enum class PixelFormat {
                RGB_UI8,
                RGBA_UI8,
                RGB_UI16,
                RGBA_UI16
            };

            // TODO(Matthew): Implement.
            //bool load(const char* filepath);

            bool save(const char* filepath, const void* data, ui32v2 dimensions, PixelFormat format);
        }
    }
}
namespace spio = SecretProject::io;

#endif // !defined(SP_IO_ImageIO_h__)
