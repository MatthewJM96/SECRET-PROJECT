/**
 * @file TextAlign.hpp
 * @brief Provides a text algin enum.
 */

#pragma once

#if !defined(SP_Graphics_TextAlign_h__)
#define SP_Graphics_TextAlign_h__

#include <type_traits>

#include "types.h"

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

#endif // !SP_Graphics_TextAlign_h__
