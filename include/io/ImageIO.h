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
            enum class PixelFormat : ui8 {
                RGB_UI8 = 0,
                RGBA_UI8,
                RGB_UI16,
                RGBA_UI16,
                SENTINEL
            };

            // TODO(Matthew): Error codes for fail states.

            namespace Binary {
                bool load(const char* filepath, void*& data, ui32v2& dimensions, PixelFormat& format);

                bool save(const char* filepath, const void* data, ui32v2 dimensions, PixelFormat format);
            }

            namespace PNG {
                // TODO(Matthew): Implement.
                //bool load(const char* filepath);

                bool save(const char* filepath, const void* data, ui32v2 dimensions, PixelFormat format);
            }
        }
    }
}
namespace spio = SecretProject::io;

#endif // !defined(SP_IO_ImageIO_h__)
