/**
 * @file WordWrap.hpp
 * @brief Provides a word wrap enum and associated functions.
 */

#pragma once

#if !defined(SP_Graphics_WordWrap_h__)
#define SP_Graphics_WordWrap_h__

#include <type_traits>

#include "types.h"

namespace SecretProject {
    namespace graphics {
        /**
         * @brief This enum is of the various ways in which text may be wrapped.
         *
         * QUICK is the quickest of the wrap modes, where it breaks immediately before the first character to exceed the rectangle.
         *
         * GREEDY only breaks on whitespace or \n characters. While quicker than MINIMUM_RAGGEDNESS, it has the less uniformity of line lengths.
         *
         * MINIMUM_RAGGEDNESS breaks on the same characters as GREEDY but seeks to minimise the difference in line lengths rather than time taken
         *     to calculate.
         */
        enum class WordWrap {
            NONE,
            QUICK,
            GREEDY,
            MINIMUM_RAGGEDNESS
        };
    }
}
namespace spg = SecretProject::graphics;

#endif // !SP_Graphics_WordWrap_h__
