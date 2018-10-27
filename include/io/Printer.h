/**
 * A helper file for printing to console.
 * 
 * Handles indentation, colouring, and various other formatting aspects.
 */

#include "types.hpp"

#pragma once

namespace ATextAdventure {
    namespace io {
        // TODO(Matthew): Ensure these colours map numerically to the colour indices in MS console API (we will just directly write RGB to console on Linux).
        Colour3 Colours[16] = {
            // TODO(Matthew): Select a colour palette of (upto) 16 colours.
            { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
            { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
            { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
            { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
        };

        // TODO(Matthew): Update these names when the colour palette has been chosen.
        enum class ColourMap {
            BLACK = 0,
            RED,
            GREEN,
            BLUE,
            YELLOW,
            MAGENTA,
            CYAN,
            WHITE,
            ETC,
            ETC2,
            ETC3,
            ETC4,
            ETC5,
            ETC6,
            ETC7,
            ETC8
        };

        // TODO(Matthew): Write functions to set colour palette and to set/reset colour selection when writing.

        class Printer {
        public:

        };
    }
}
