#include "stdafx.h"
#include "graphics/Font.h"

/**
 * @brief Determines the next power of 2 after the given value and returns it.
 *
 * @param The value to determine the following power of 2 for.
 *
 * @return The power of 2 determined.
 */
ui32 nextPower2(ui32 value) {
    // This is a rather lovely bit manipulation function.
    // Essentially, all we're doing in this is up until the return
    // statement we take a value like 0110110000110101101 and change 
    // it to become 0111111111111111111 so that when we add 1 (i.e.
    // 0000000000000000001) it becomes 1000000000000000000 -> the
    // next power of 2!
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return ++value;
}

spg::FontInstanceHash spg::hash(FontSize size, FontStyle style, FontRenderStyle renderStyle) {
    FontInstanceHash hash = 0;

    // By ensuring FontInstanceHash has more bits that the sum of all three of the
    // provided values we can generate the hash simply by shifting the bits of style
    // and render style such that none of the three overlap.
    hash += static_cast<FontInstanceHash>(size);
    hash += static_cast<FontInstanceHash>(style)       << (sizeof(FontSize) * 8);
    hash += static_cast<FontInstanceHash>(renderStyle) << ((sizeof(FontSize) + sizeof(FontStyle)) * 8);

    return hash;
}

spg::Font::Font() :
    m_filepath(nullptr),
    m_start(0), m_end(0),
    m_defaultSize(0)
{ /* Empty. */ }

void spg::Font::init(const char* filepath, char start, char end) {
    m_filepath = filepath;
    m_start    = start;
    m_end      = end;
}

void spg::Font::dispose() {
    for (auto& fontInstance : m_fontInstances) {
        if (fontInstance.second.texture != 0) {
            glDeleteTextures(1, &fontInstance.second.texture);
        }
        if (fontInstance.second.glyphs != nullptr) {
            delete[] fontInstance.second.glyphs;
        }
    }

    FontInstanceMap().swap(m_fontInstances);
}

bool spg::Font::generate(       FontSize size,
                                FontSize padding,
                               FontStyle style       /*= FontStyle::NORMAL*/,
                         FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    // Make sure this is a new instance we are generating.
    if (getFontInstance(size, style, renderStyle) != NIL_FONT_INSTANCE) return false;

    // This is the font instance we will build up as we generate the texture atlas.
    FontInstance fontInstance{};
    // Create the glyphs array for this font instance.
    fontInstance.glyphs = new Glyph[m_end - m_start];

    // Open the font and check we didn't fail.
    TTF_Font* font = TTF_OpenFont(m_filepath, size);
    if (font == nullptr) return false;

    // Set the font style.
    TTF_SetFontStyle(font, static_cast<int>(style));

    // Store the height of the tallest glyph for the given font size.
    fontInstance.height = TTF_FontHeight(font);

    // For each character, we are going to get the glyph metrics - that is the set of
    // properties that constitute begin and end positions of the glyph - and calculate
    // each glyph's size.
    {
        size_t i = 0;
        for (char c = m_start; c <= m_end; ++c) {
            fontInstance.glyphs[i].character = c;

            i32v4 metrics;

            TTF_GlyphMetrics(font, c, &metrics.x, &metrics.y,
                                    &metrics.z, &metrics.w, nullptr);

            fontInstance.glyphs[i].size.x = metrics.z - metrics.x;
            fontInstance.glyphs[i].size.y = metrics.w - metrics.y;

            ++i;
        }
    }

    // Our texture atlas of all the glyphs in the font is going to have multiple rows.
    // We want to make this texture as small as possible in memory, so we now do some
    // preprocessing in order to find the number of rows that minimises the area of
    // the atlas (equivalent to the amount of data that will be used up by it).
    ui32 rowCount     = 1;
    ui32 bestWidth    = 0;
    ui32 bestHeight   = 0;
    ui32 bestArea     = std::numeric_limits<ui32>::max();
    ui32 bestRowCount = 0;
    Row* bestRows = nullptr;
    while (rowCount <= static_cast<ui32>(m_end - m_start)) {
        // Generate rows for the current row count, getting the width and height of the rectangle
        // they form.
        ui32 currentWidth, currentHeight;
        Row* currentRows = generateRows(fontInstance.glyphs, rowCount, padding, currentWidth, currentHeight);

        // There are benefits of making the texture larger to match power of 2 boundaries on
        // width and height.
        currentWidth  = nextPower2(currentWidth);
        currentHeight = nextPower2(currentHeight);

        // TODO(Matthew): Pretty sure that this isn't actually getting the maximum
        //                size of one dimension, rather the maximum area allowed
        //                after squaring this value. If so, we should revisit this
        //                loop!
        // Get maximum texture size allowed by implementation.
        GLint maxTextureSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

        // If current width exceeds the maximum OpenGL texture size (and current height does not),
        // then try adding another row.
        if (currentWidth > static_cast<ui32>(maxTextureSize) && currentHeight < static_cast<ui32>(maxTextureSize)) {
            ++rowCount;
            delete[] currentRows;
            continue;
        }

        // If the area of the rectangle drawn out by the rows generated is less than the previous
        // best area, then we have a new candidate!
        if (currentWidth * currentHeight < bestArea) {
            if (bestRows) delete[] bestRows;
            bestRows     = currentRows;
            bestWidth    = currentWidth;
            bestHeight   = currentHeight;
            bestRowCount = rowCount;
            bestArea     = bestWidth * bestHeight;
            ++rowCount;

            // If current height exceeds the maximum OpenGL texture size then there's no point
            // considering adding another row.
            if (currentHeight > static_cast<ui32>(maxTextureSize)) {
                delete[] currentRows;
                break;
            }
        } else {
            // Area has increased, break out as going forwards it's likely area will only continue
            // to increase.
            delete[] currentRows;
            break;
        }
    }

    // Make sure we actually have rows to use.
    if (bestRows == nullptr) return false;

    // Generate & bind the texture we will put each glyph into.
    glGenTextures(1, &fontInstance.texture);
    glBindTexture(GL_TEXTURE_2D, fontInstance.texture);
    // Set the texture's size and pixel format.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bestWidth, bestHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // This represents the current V-coordinate we are into the texture.
    //    UV are the coordinates we use for textures (i.e. the X & Y coords of the pixels).
    ui32 currentV = padding;
    // Loop over all of the rows, for each going through and drawing each glyph,
    // adding it to our texture.
    for (size_t rowIndex = 0; rowIndex < bestRowCount; ++rowIndex) {
        // This represents the current U-coordinate we are into the texture.
        ui32 currentU = padding;
        for (size_t glyphIndex = 0; glyphIndex < bestRows[rowIndex].second.size(); ++glyphIndex) {
            ui32 charIndex = bestRows[rowIndex].second[glyphIndex];

            // Determine which render style we are to use and draw the glyph.
            SDL_Surface* glyphSurface = nullptr;
            switch(renderStyle) {
                case FontRenderStyle::SOLID:
                    glyphSurface = TTF_RenderGlyph_Solid(font, static_cast<ui16>(m_start + charIndex), { 255, 255, 255, 255 });
                    break;
                case FontRenderStyle::BLENDED:
                    glyphSurface = TTF_RenderGlyph_Blended(font, static_cast<ui16>(m_start + charIndex), { 255, 255, 255, 255 });
                    break;
            }

            // Stitch the glyph we just generated into our texture.
            glTexSubImage2D(GL_TEXTURE_2D, 0, currentU, currentV, glyphSurface->w, glyphSurface->h, GL_BGRA, GL_UNSIGNED_BYTE, glyphSurface->pixels);

            // Build the UV dimensions for the glyph.
            fontInstance.glyphs[charIndex].uvDimensions.x = (currentU / bestWidth);
            fontInstance.glyphs[charIndex].uvDimensions.y = (currentV / bestHeight);
            fontInstance.glyphs[charIndex].uvDimensions.z = (glyphSurface->w / bestWidth);
            fontInstance.glyphs[charIndex].uvDimensions.w = (glyphSurface->h / bestHeight);

            // Free the glyph "surface".
            SDL_FreeSurface(glyphSurface);
            glyphSurface = nullptr;

            // Update currentU.
            currentU += glyphSurface->w + padding;
        }
        // Update currentV.
        currentV += bestRows[rowIndex].first + padding;
    }

    // Clean up.
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] bestRows;

    TTF_CloseFont(font);

    // Insert our font instance.
    m_fontInstances.emplace(std::make_pair(hash(size, style, renderStyle), fontInstance));

    return true;
}

spg::FontInstance spg::Font::getFontInstance(       FontSize size,
                                                   FontStyle style       /*= FontStyle::NORMAL*/,
                                             FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    try {
        return m_fontInstances.at(hash(size, style, renderStyle));
    } catch (std::out_of_range e) {
        return NIL_FONT_INSTANCE;
    }
}

spg::Font::Row* spg::Font::generateRows(Glyph* glyphs, ui32 rowCount, FontSize padding, ui32& width, ui32& height) {
    // Create some arrays for the rows, their widths and max height of a glyph within each of them.
    //    Max heights are stored inside Row - it is a pair of max height and a vector of glyph indices.
    Row*  rows          = new Row[rowCount]();
    ui32* currentWidths = new ui32[rowCount]();

    width  = padding;
    height = padding * rowCount + padding;
    // Initialise our arrays of widths and max heights.
    for (size_t i = 0; i < rowCount; ++i) {
        currentWidths[i] = padding;
        rows[i].first    = 0;
    }

    // For each character, we now determine which row to put it in, updating the width and
    // height variables as we go.
    for (ui32 i = 0; i < static_cast<ui32>(m_end - m_start); ++i) {
        // Determine which row currently has the least width: this is the row we will add
        // the currently considered glyph to.
        size_t bestRow = 0;
        for (size_t j = 1; j < rowCount; ++j) {
            // If row with index j is not as wide as the current least-wide row then it becomes
            // the new least-wide row!
            if (currentWidths[bestRow] > currentWidths[j]) bestRow = j;
        }

        // Update the width of the row we have chosen to add the glyph to.
        currentWidths[bestRow] += glyphs[i].size.x + padding;

        // Update the overall width of the rectangle the rows form,
        // if our newly enlarged row exceeds it.
        if (width < currentWidths[bestRow]) width = currentWidths[bestRow];

        // Update the max height of our row if the new glyph exceeds it, and update
        // the height of the rectange the rows form.
        if (rows[bestRow].first < glyphs[i].size.y) {
            height -= rows[bestRow].first;
            height += glyphs[i].size.y;
            rows[bestRow].first = glyphs[i].size.y;
        }

        rows[bestRow].second.push_back(i);
    }

    // Clear up memory.
    delete[] currentWidths;

    // Return the rows we've built up!
    return rows;
}

void spg::FontCache::dispose() {
    // Dispose the cached fonts.
    for (auto& font : m_fonts) {
        font.second.dispose();
    }

    // Empty our map of fonts.
    Fonts().swap(m_fonts);
}

bool spg::FontCache::registerFont(const char* name, const char* filepath, char start, char end) {
    // Try to emplace a new Font object with the given name.
    auto [it, added] = m_fonts.try_emplace(name, Font());
    // If we added it, then initialise the Font object.
    if (added) {
        m_fonts.at(name).init(filepath, start, end);
        return true;
    }
    return false;
}

bool spg::FontCache::registerFont(const char* name, const char* filepath) {
    // Try to emplace a new Font object with the given name.
    auto [it, added] = m_fonts.try_emplace(name, Font());
    // If we added it, then initialise the Font object.
    if (added) {
        m_fonts.at(name).init(filepath);
        return true;
    }
    return false;
}

spg::FontInstance spg::FontCache::fetchFontInstance(    const char* name,
                                                           FontSize size,
                                                          FontStyle style       /*= FontStyle::NORMAL*/,
                                                    FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    // Make sure a font exists with the given name.
    auto font = m_fonts.find(name);
    if (font == m_fonts.end()) return NIL_FONT_INSTANCE;

    // Generate the specified font instance if it doesn't exist.
    font->second.generate(size, style, renderStyle);

    // Return the font instance.
    return font->second.getFontInstance(size, style, renderStyle);
}

spg::FontInstance spg::FontCache::fetchFontInstance(    const char* name,
                                                          FontStyle style       /*= FontStyle::NORMAL*/,
                                                    FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    // Make sure a font exists with the given name.
    auto font = m_fonts.find(name);
    if (font == m_fonts.end()) return NIL_FONT_INSTANCE;

    // Generate the specified font instance if it doesn't exist.
    font->second.generate(style, renderStyle);

    // Return the font instance.
    return font->second.getFontInstance(style, renderStyle);
}

bool operator==(const spg::FontInstance& lhs, const spg::FontInstance& rhs) {
    return (lhs.texture == rhs.texture &&
            lhs.height  == rhs.height  &&
            lhs.glyphs  == rhs.glyphs);
}
bool operator!=(const spg::FontInstance& lhs, const spg::FontInstance& rhs) {
    return !(lhs == rhs);
}

// These are just a set of functions to let us use bit-masking for FontStyle.
//     That is to say, we can do things like:
//         FontStyle::BOLD | FontStyle::ITALIC
//     in order to specify we want a font instance that is bold AND italic!
spg::FontStyle  operator~  (spg::FontStyle rhs) {
    return static_cast<spg::FontStyle>(
        ~static_cast<std::underlying_type<spg::FontStyle>::type>(rhs)
    );
}
spg::FontStyle  operator|  (spg::FontStyle lhs,  spg::FontStyle rhs) {
    return static_cast<spg::FontStyle>(
        static_cast<std::underlying_type<spg::FontStyle>::type>(lhs) |
        static_cast<std::underlying_type<spg::FontStyle>::type>(rhs)
    );
}
spg::FontStyle  operator&  (spg::FontStyle lhs,  spg::FontStyle rhs) {
    return static_cast<spg::FontStyle>(
        static_cast<std::underlying_type<spg::FontStyle>::type>(lhs) &
        static_cast<std::underlying_type<spg::FontStyle>::type>(rhs)
    );
}
spg::FontStyle  operator^  (spg::FontStyle lhs,  spg::FontStyle rhs) {
    return static_cast<spg::FontStyle>(
        static_cast<std::underlying_type<spg::FontStyle>::type>(lhs) ^
        static_cast<std::underlying_type<spg::FontStyle>::type>(rhs)
    );
}
spg::FontStyle& operator|= (spg::FontStyle& lhs, spg::FontStyle rhs) {
    lhs = static_cast<spg::FontStyle>(
        static_cast<std::underlying_type<spg::FontStyle>::type>(lhs) |
        static_cast<std::underlying_type<spg::FontStyle>::type>(rhs)
    );

    return lhs;
}
spg::FontStyle& operator&= (spg::FontStyle& lhs, spg::FontStyle rhs) {
    lhs = static_cast<spg::FontStyle>(
        static_cast<std::underlying_type<spg::FontStyle>::type>(lhs) &
        static_cast<std::underlying_type<spg::FontStyle>::type>(rhs)
    );

    return lhs;
}
spg::FontStyle& operator^= (spg::FontStyle& lhs, spg::FontStyle rhs) {
    lhs = static_cast<spg::FontStyle>(
        static_cast<std::underlying_type<spg::FontStyle>::type>(lhs) ^
        static_cast<std::underlying_type<spg::FontStyle>::type>(rhs)
    );

    return lhs;
}
