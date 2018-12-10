/**
 * @file Gradients.hpp
 * @brief Provides a gradient enum and associated functions.
 */

#pragma once

#if !defined(SP_Graphics_Gradients_h__)
#define SP_Graphics_Gradients_h__

#include <type_traits>

#include "types.h"

enum class Gradient {
    NONE,
    LEFT_TO_RIGHT,
    TOP_TO_BOTTOM,
    TOP_LEFT_TO_BOTTOM_RIGHT,
    TOP_RIGHT_TO_BOTTOM_LEFT
};

template <typename T = f32, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
colour3 lerp(colour3 c1, colour3 c2, T ratio) {
    return colour3{
        static_cast<ui8>(static_cast<T>(c1.r) * (1.0 - ratio) + static_cast<T>(c2.r) * ratio),
        static_cast<ui8>(static_cast<T>(c1.g) * (1.0 - ratio) + static_cast<T>(c2.g) * ratio),
        static_cast<ui8>(static_cast<T>(c1.b) * (1.0 - ratio) + static_cast<T>(c2.b) * ratio)
    };
}

template <typename T = f32, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
colour4 lerp(colour4 c1, colour4 c2, T ratio) {
    return colour4{
        static_cast<ui8>(static_cast<T>(c1.r) * (1.0 - ratio) + static_cast<T>(c2.r) * ratio),
        static_cast<ui8>(static_cast<T>(c1.g) * (1.0 - ratio) + static_cast<T>(c2.g) * ratio),
        static_cast<ui8>(static_cast<T>(c1.b) * (1.0 - ratio) + static_cast<T>(c2.b) * ratio),
        static_cast<ui8>(static_cast<T>(c1.a) * (1.0 - ratio) + static_cast<T>(c2.a) * ratio)
    };
}

#endif // !SP_Graphics_Gradients_h__
