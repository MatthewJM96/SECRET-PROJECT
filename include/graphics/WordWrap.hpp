/**
 * @file WordWrap.hpp
 * @brief Provides a word wrap enum and associated functions.
 */

#pragma once

#if !defined(SP_Graphics_WordWrap_h__)
#define SP_Graphics_WordWrap_h__

#include <type_traits>

#include "types.h"

/**
 * @brief This enum is of the various ways in which text may be wrapped.
 *
 * GREEDY is the quickest of the two wrapped modes, but has the least uniformity of line lengths.
 * MINIMUM_RAGGEDNESS seeks to minimise the difference in line lengths rather than time taken to calculate.
 */
enum class WordWrap {
    NONE,
    GREEDY,
    MINIMUM_RAGGEDNESS
};

#endif // !SP_Graphics_WordWrap_h__
