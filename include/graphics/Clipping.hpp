/**
 * @file Clipping.hpp
 * @brief Provides some helper functions.
 */

#pragma once

#if !defined(SP_Graphics_Clipping_h__)
#define SP_Graphics_Clipping_h__

#include <type_traits>

#include "types.h"

namespace SecretProject {
    namespace graphics {
        /**
         * @brief Clips an object with the given position, size and UV coordinates & size, such that
         * it entirely fits within the given clip rectangle.
         *
         * @param clip The clip rectangle to clip to.
         * @param position The position of the object to clip.
         * @param size The size of the object to clip.
         * @param uvDimensions The UV coordinates & size of the object to clip.
         *
         * @return True if any of the properties of the object were changed, false otherwise.
         */
        bool clip(const f32v4& clip, f32v2& position, f32v2& size, f32v4& uvDimensions) {
            // Flag of if anything has changed.
            bool changed = false;

            // Check if the object we are clipping extends to the left of the clip rectangle.
            if (position.x < clip.x) {
                // Work out by how much the object extends.
                f32 delta = clip.x - position.x;
                f32 ratio = delta / size.x;

                // Update the uvDimensions using the proportion of the object that extends.
                uvDimensions.x += uvDimensions.z * ratio;
                uvDimensions.z -= uvDimensions.z * ratio;

                // Update the position and size of the object.
                position.x = clip.x;
                size.x    -= delta;

                // Something has changed.
                changed = true;
            }

            // Check if the object we are clipping extends to the right of the clip rectangle.
            if (position.x + size.x > clip.x + clip.z) {
                // Work out by how much the object extends.
                f32 delta = position.x + size.x - (clip.x + clip.z);
                f32 ratio = delta / size.x;

                // Update the uvDimensions using the proportion of the object that extends.
                uvDimensions.z -= uvDimensions.z * ratio;

                // Update the size of the object.
                size.x -= delta;

                // Something has changed.
                changed = true;
            }

            // Check if the object we are clipping extends above the clip rectangle.
            if (position.y < clip.y) {
                // Work out by how much the object extends.
                f32 delta = clip.y - position.y;
                f32 ratio = delta / size.y;

                // Update the uvDimensions using the proportion of the object that extends.
                uvDimensions.y += uvDimensions.w * ratio;
                uvDimensions.w -= uvDimensions.w * ratio;

                // Update the position and size of the object.
                position.y = clip.y;
                size.y    -= delta;

                // Something has changed.
                changed = true;
            }

            // Check if the object we are clipping extends below the clip rectangle.
            if (position.y + size.y > clip.y + clip.w) {
                // Work out by how much the object extends.
                f32 delta = position.y + size.y - (clip.y + clip.w);
                f32 ratio = delta / size.y;

                // Update the uvDimensions using the proportion of the object that extends.
                uvDimensions.w -= uvDimensions.w * ratio;

                // Update the position and size of the object.
                size.y -= delta;

                // Something has changed.
                changed = true;
            }

            return changed;
        }
    }
}
namespace spg = SecretProject::graphics;

#endif // !SP_Graphics_Clipping_h__
