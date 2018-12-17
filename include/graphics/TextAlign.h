/**
 * @file TextAlign.h
 * @brief Provides a text algin enum.
 */

#pragma once

#if !defined(SP_Graphics_TextAlign_h__)
#define SP_Graphics_TextAlign_h__

#include <type_traits>

#include "types.h"

namespace SecretProject {
    namespace graphics {
        /**
         * @brief This enum is of the various ways in which text may be aligned within a rectangle.
         */
        enum class TextAlign {
            NONE,
            CENTER_LEFT,
            TOP_LEFT,
            TOP_CENTER,
            TOP_RIGHT,
            CENTER_RIGHT,
            BOTTOM_RIGHT,
            BOTTOM_CENTER,
            BOTTOM_LEFT,
            CENTER_CENTER
        };

        /**
         * @brief Calculates the offsets needed for a specific line of text.
         *
         * @param align The alignment of the text.
         * @param rect The rectangle in which the text will be drawn.
         * @param height The height of the entire body of text to be drawn.
         * @param length The length of the line for which the offset is being calculated.
         *
         * @return The offset calculated.
         */
        f32v2 calculateOffset(TextAlign align, const f32v4& rect, f32 height, f32 length);
    }
}
namespace spg = SecretProject::graphics;

#endif // !SP_Graphics_TextAlign_h__
