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
        };

        // TODO(Matthew): Write functions to set colour palette and to set/reset colour selection when writing.
    }
}
