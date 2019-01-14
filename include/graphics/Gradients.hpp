/**
 * @file Gradients.hpp
 * @brief Provides a gradient enum and associated functions.
 */

#pragma once

#if !defined(SP_Graphics_Gradients_h__)
#define SP_Graphics_Gradients_h__

#include <type_traits>

#include "types.h"

namespace SecretProject {
    namespace graphics {
        /**
         * @brief Provides an enumeration of the possible gradient styles.
         */
        enum class Gradient {
            NONE,
            LEFT_TO_RIGHT,
            TOP_TO_BOTTOM,
            TOP_LEFT_TO_BOTTOM_RIGHT,
            TOP_RIGHT_TO_BOTTOM_LEFT
        };

        /**
         * @brief Provides linear interpolation between two colours to a given mix.
         *
         * @tparam T The type to use for the ratio for interpolating - this must be a
         * floating-point type.
         *
         * @param c1 The first colour to use for the lerp.
         * @param c2 The second colour to use for the lerp.
         * @param ratio The ratio in which to mix the two colours.
         *
         * @return The mix of the two colours determined. At a ratio of 0 the first colour is
         * returned and at a ratio of 1 the second colour is returned. Values between 0 and 1
         * return a mix between the two colours in a linear fashion.
         */
        template <typename T = f32, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
        colour3 lerp(colour3 c1, colour3 c2, T ratio) {
            return {
                static_cast<ui8>(static_cast<T>(c1.r) * (1.0 - ratio) + static_cast<T>(c2.r) * ratio),
                static_cast<ui8>(static_cast<T>(c1.g) * (1.0 - ratio) + static_cast<T>(c2.g) * ratio),
                static_cast<ui8>(static_cast<T>(c1.b) * (1.0 - ratio) + static_cast<T>(c2.b) * ratio)
            };
        }

        /**
         * @brief Provides linear interpolation between two colours to a given mix.
         *
         * @tparam T The type to use for the ratio for interpolating - this must be a
         * floating-point type.
         *
         * @param c1 The first colour to use for the lerp.
         * @param c2 The second colour to use for the lerp.
         * @param ratio The ratio in which to mix the two colours.
         *
         * @return The mix of the two colours determined. At a ratio of 0 the first colour is
         * returned and at a ratio of 1 the second colour is returned. Values between 0 and 1
         * return a mix between the two colours in a linear fashion.
         */
        template <typename T = f32, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
        colour4 lerp(colour4 c1, colour4 c2, T ratio) {
            return {
                static_cast<ui8>(static_cast<T>(c1.r) * (1.0 - ratio) + static_cast<T>(c2.r) * ratio),
                static_cast<ui8>(static_cast<T>(c1.g) * (1.0 - ratio) + static_cast<T>(c2.g) * ratio),
                static_cast<ui8>(static_cast<T>(c1.b) * (1.0 - ratio) + static_cast<T>(c2.b) * ratio),
                static_cast<ui8>(static_cast<T>(c1.a) * (1.0 - ratio) + static_cast<T>(c2.a) * ratio)
            };
        }
    }
}
namespace spg = SecretProject::graphics;

#endif // !SP_Graphics_Gradients_h__
