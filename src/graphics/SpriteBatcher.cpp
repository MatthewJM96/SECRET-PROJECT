#include "stdafx.h"
#include "graphics/SpriteBatcher.h"

#include "graphics/Clipping.hpp"
#include "graphics/Font.h"

#define VERTICES_PER_QUAD 4
#define INDICES_PER_QUAD  6

spg::SpriteBatcher::SpriteBatcher() :
    m_vao(0), m_vbo(0), m_ibo(0),
    m_usageHint(GL_STATIC_DRAW),
    m_indexCount(0)
{
    /* Empty */
}
spg::SpriteBatcher::~SpriteBatcher() {
    /* Empty */
}

void spg::SpriteBatcher::init(FontCache* fontCache, GLenum usageHint /*= GL_STATIC_DRAW*/) {
    m_fontCache = fontCache;
    m_usageHint = usageHint;

    /*****************************\
     * Create a default shader . *
    \*****************************/

    // Create a default shader program.
    m_defaultShader.init();

    // Set each attribute's corresponding index.
    m_defaultShader.setAttribute("vPosition",         SpriteShaderAttribID::POSITION);
    m_defaultShader.setAttribute("vRelativePosition", SpriteShaderAttribID::RELATIVE_POSITION);
    m_defaultShader.setAttribute("vUVDimensions",     SpriteShaderAttribID::UV_DIMENSIONS);
    m_defaultShader.setAttribute("vColour",           SpriteShaderAttribID::COLOUR);

    // TODO(Matthew): Handle errors.
    // Add the shaders to the program.
    m_defaultShader.addShaders("shaders/DefaultSprite.vert", "shaders/DefaultSprite.frag");

    // Link program (i.e. send to GPU).
    m_defaultShader.link();

    // Set default shader as active shader.
    m_activeShader = &m_defaultShader;

    /******************\
     * Create the VAO *
    \******************/

    // Gen the vertex array object and bind it.
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Generate the associated vertex & index buffers - these are the bits of memory that will be populated within the GPU storing information
    // about the graphics we want to draw.
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    // Bind those buffers
    //    OpenGL generally follows a pattern of generate an ID corresponding to some memory on the GPU, bind said memory, link some properties to them 
    //    (such as how the data in the memory correspond to variables inside our shader programs).
    glBindBuffer(GL_ARRAY_BUFFER,         m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    // Enable the attributes in our shader.
    m_defaultShader.enableVertexAttribArrays();

    // Connect the vertex attributes in the shader (e.g. vPosition) to its corresponding chunk of memory inside the SpriteVertex struct.
    //     We first tell OpenGL the ID of the attribute within the shader (as we set earlier), then the number of values and their type.
    //
    //     After that, we tell OpenGL whether that data should be normalised (e.g. unsigned bytes that need normalising will be converted 
    //     to a float divided through by 255.0f and by OpenGL - so that colours, e.g., are represented by values between 0.0f and 1.0f 
    //     per R/G/B/A channel rather than the usual 0 to 255).
    //
    //     We then pass the size of the data representing a vertex followed by how many bytes into that data the value is stored - we use offset rather than 
    //     manually writing this to give us flexibility in changing the order of the SpriteVertex struct.
    glVertexAttribPointer(SpriteShaderAttribID::POSITION,          3, GL_FLOAT,         false, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, position)));
    glVertexAttribPointer(SpriteShaderAttribID::RELATIVE_POSITION, 2, GL_FLOAT,         false, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, relativePosition)));
    glVertexAttribPointer(SpriteShaderAttribID::UV_DIMENSIONS,     4, GL_FLOAT,         false, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, uvDimensions)));
    glVertexAttribPointer(SpriteShaderAttribID::COLOUR,            4, GL_UNSIGNED_BYTE, true,  sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, colour)));

    // Clean everything up, unbinding each of our buffers and the vertex array.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /***********************************\
     * Create a default white texture. *
    \***********************************/

    // Generate and bind texture.
    glGenTextures(1, &m_defaultTexture);
    glBindTexture(GL_TEXTURE_2D, m_defaultTexture);

    // Set texture to be just a 1x1 image of a pure white pixel.
    ui32 pix = 0xffffffff;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pix);

    // Set texture parameters to repeat our pixel as needed and to not do any averaging of pixels.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,     GL_REPEAT);

    // Unbind our complete texture.
    glBindTexture(GL_TEXTURE_2D, 0);
}

void spg::SpriteBatcher::dispose() {
    // Clean up buffer objects before vertex array.
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }

    if (m_ibo != 0) {
        glDeleteBuffers(1, &m_ibo);
        m_ibo = 0;
        m_indexCount = 0;
    }

    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    // Delete our default texture.
    if (m_defaultTexture != 0) {
        glDeleteTextures(1, &m_defaultTexture);
        m_defaultTexture = 0;
    }

    // Reset properties and stored sprites & batches.
    m_usageHint  = GL_STATIC_DRAW;
    m_indexCount = 0;

    Sprites().swap(m_sprites);
    SpritePtrs().swap(m_spritePtrs);
    Batches().swap(m_batches);
}

void spg::SpriteBatcher::reserve(size_t count) {
    if (m_sprites.size() < count) {
        m_sprites.reserve(count);
        m_spritePtrs.reserve(count);
    }
}

void spg::SpriteBatcher::begin() {
    m_sprites.clear();
    m_batches.clear();
}

void spg::SpriteBatcher::draw(Sprite&& sprite) {
    m_sprites.emplace_back(std::forward<Sprite>(sprite));

    Sprite& spriteRef = m_sprites.back();
    if (spriteRef.texture == 0) {
        spriteRef.texture = m_defaultTexture;
    }
}

void spg::SpriteBatcher::draw( QuadBuilder builder,
                                    GLuint texture,
                              const f32v2& position,
                              const f32v2& size,
                                   colour4 c1       /*= { 255, 255, 255, 255 }*/,
                                   colour4 c2       /*= { 255, 255, 255, 255 }*/,
                                  Gradient gradient /*= Gradient::NONE*/,
                                       f32 depth    /*= 0.0f*/,
                              const f32v4& uvRect   /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/) {
    m_sprites.emplace_back(Sprite{
        builder,
        texture == 0 ? m_defaultTexture : texture,
        position,
        size,
        depth,
        uvRect,
        c1,
        c2,
        gradient
    });
}

void spg::SpriteBatcher::draw(      GLuint texture,
                              const f32v2& position,
                              const f32v2& size,
                                   colour4 c1       /*= { 255, 255, 255, 255 }*/,
                                   colour4 c2       /*= { 255, 255, 255, 255 }*/,
                                  Gradient gradient /*= Gradient::NONE*/,
                                       f32 depth    /*= 0.0f*/,
                              const f32v4& uvRect   /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/) {
    m_sprites.emplace_back(Sprite{
        &buildQuad,
        texture == 0 ? m_defaultTexture : texture,
        position,
        size,
        depth,
        uvRect,
        c1,
        c2,
        gradient
    });
}

void spg::SpriteBatcher::drawString(    const char* str,
                                              f32v4 rect,
                                       StringSizing sizing,
                                            colour4 tint,
                                        const char* fontName,
                                           FontSize fontSize,
                                          TextAlign align       /*= TextAlign::TOP_LEFT*/,
                                           WordWrap wrap        /*= WordWrap::NONE*/,
                                                f32 depth       /*= 0.0f*/,
                                          FontStyle style       /*= FontStyle::NORMAL*/,
                                    FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    drawString(str, rect, sizing, tint, m_fontCache->fetchFontInstance(fontName, fontSize, style, renderStyle), align, wrap, depth);
}
void spg::SpriteBatcher::drawString(    const char* str,
                                              f32v4 rect,
                                       StringSizing sizing,
                                            colour4 tint,
                                        const char* fontName,
                                          TextAlign align       /*= TextAlign::TOP_LEFT*/,
                                           WordWrap wrap        /*= WordWrap::NONE*/,
                                                f32 depth       /*= 0.0f*/,
                                          FontStyle style       /*= FontStyle::NORMAL*/,
                                    FontRenderStyle renderStyle /*= FontRenderStyle::BLENDED*/) {
    drawString(str, rect, sizing, tint, m_fontCache->fetchFontInstance(fontName, style, renderStyle), align, wrap, depth);
}
void spg::SpriteBatcher::drawString( const char* str,
                                           f32v4 rect,
                                    StringSizing sizing,
                                         colour4 tint,
                                    FontInstance fontInstance,
                                       TextAlign align /*= TextAlign::TOP_LEFT*/,
                                        WordWrap wrap  /*= WordWrap::NONE*/,
                                             f32 depth /*= 0.0f*/) {
    if (fontInstance == NIL_FONT_INSTANCE) return;

    StringComponents components { std::make_pair(str, StringDrawProperties{ fontInstance, sizing, tint }) };

    drawString(components, rect, align, wrap, depth);
}

// Need this for centered alignments.
// struct DrawableGlyph {
//     Glyph glyph;
//     f32   xPos;
//     f32v2 scaling;
// };

void spg::SpriteBatcher::drawString(StringComponents components,
                                               f32v4 rect,
                                           TextAlign align /*= TextAlign::TOP_LEFT*/,
                                            WordWrap wrap  /*= WordWrap::NONE*/,
                                                 f32 depth /*= 0.0f*/) {
    // TODO(Matthew): Implement actual string draw. (OH GOD PLEASE HELP ME)

    (void) align;
    (void) wrap;

    // Need this for centered alignments.
    // std::vector<std::vector<DrawableGlyph>> lines;
    // lines.emplace_back();
    // TODO(Matthew): For now just implementing no word wrap with top-left alignment.
    //                 -> Complete for other wrap modes and alignments.
    // f32 currentX = 0.0f;
    f32v2 currentPos = f32v2(0.0f);
    for (auto& component : components) {
        // Simplify property names.
        const char*  str    = component.first;
        FontInstance font   = component.second.fontInstance;
        StringSizing sizing = component.second.sizing;
        colour4      tint   = component.second.tint;

        char start = font.owner->getStart();
        char end   = font.owner->getEnd();

        // Process sizing into a simple scale factor.
        f32v2 scaling;
        f32   lineHeight;
        if (sizing.kind == StringSizingKind::SCALED) {
            scaling    = sizing.scaling;
            lineHeight = static_cast<f32>(font.height) * scaling.y;
        } else {
            scaling.x  = sizing.scaleX;
            scaling.y  = sizing.targetHeight / static_cast<f32>(font.height);
            lineHeight = sizing.targetHeight;
        }
 
        // Iterate over this component's string.
        for (size_t i = 0; str[i] != '\0'; ++i) {
            char   character      = str[i];
            size_t characterIndex = static_cast<size_t>(character) - static_cast<size_t>(start);

            // If character is a new line character, add a new line and go to next character.
            if (character == '\n') {
                // Do this for centered alignments instead of incrementing currentPos.y.
                // lines.emplace_back();
                currentPos.x  = 0.0f;
                currentPos.y += lineHeight;
                continue;
            }

            // TODO(Matthew): Handle unsupported characters better?
            // If character is unsupported, skip.
            if (character < start || character > end) continue;

            // Skip non-newline characters once out of bounds of our rectangle.
            if (currentPos.x > rect.z) continue;

            // Note, we are now going to directly draw the glyph into the sprite batcher here as we are assuming an
            // anchored start position for text (TOP_LEFT alignment). Any anchored alignment can do this. However
            // centered alignments cannot as they need to know the size of the rectangle the text forms to determine
            // start position.
            {
                f32v2 size         = font.glyphs[characterIndex].size * scaling;
                f32v2 position     = currentPos + f32v2(0.0f, lineHeight - size.y);
                f32v4 uvDimensions = font.glyphs[characterIndex].uvDimensions;

                clip(rect, position, size, uvDimensions);

                if (size.x > 0.0f && size.y > 0.0f) {
                    draw(font.texture, position, size, tint,
                            { 255, 255, 255, 255 }, Gradient::NONE, depth, uvDimensions);
                } else {
                    continue;
                }
            }

            // For centered alignments, we would have to emplace back a drawable glyph and then draw later.
            // lines.back().emplace_back(DrawableGlyph{ font.glyphs[characterIndex], currentPos.x, scaling });

            f32 characterWidth = font.glyphs[characterIndex].size.x * scaling.x;
            currentPos.x += characterWidth;
        }
    }
}

void spg::SpriteBatcher::end(SpriteSortMode sortMode /*= SpriteSortMode::TEXTURE*/) {
    // Reserve the right amount of space to then assign a pointer for each sprite.
    if (m_spritePtrs.capacity() < m_sprites.size()) {
        m_spritePtrs.reserve(m_sprites.size());
    } else {
        m_spritePtrs.resize(m_sprites.size());
    }
    for (size_t i = 0; i < m_sprites.size(); ++i) {
        m_spritePtrs[i] = &m_sprites[i];
    }

    // Sort the sprites - we sort the vector of pointers only for speed.
    sortSprites(sortMode);

    // Generate the batches to use for draw calls.
    generateBatches();
}

bool spg::SpriteBatcher::setShader(GLSLProgram* shader /*= nullptr*/) {
    if (shader == nullptr) {
        m_activeShader = &m_defaultShader;
    } else {
        if (!shader->isInitialised()) return false;

        if (!shader->isLinked()) {
            shader->setAttribute("vPosition",         SpriteShaderAttribID::POSITION);
            shader->setAttribute("vRelativePosition", SpriteShaderAttribID::RELATIVE_POSITION);
            shader->setAttribute("vUVDimensions",     SpriteShaderAttribID::UV_DIMENSIONS);
            shader->setAttribute("vColour",           SpriteShaderAttribID::COLOUR);

            if (shader->link() != ShaderLinkResult::SUCCESS) return false;
        }

        m_activeShader = shader;
    }

    return true;
}

void spg::SpriteBatcher::render(const f32m4& worldProjection, const f32m4& viewProjection) {
        // Activate the shader.
        m_activeShader->use();

        // Upload our projection matrices.
        glUniformMatrix4fv(m_activeShader->getUniformLocation("WorldProjection"), 1, false, &worldProjection[0][0]);
        glUniformMatrix4fv(m_activeShader->getUniformLocation("ViewProjection"),  1, false, &viewProjection[0][0]);

        // Bind our vertex array.
        glBindVertexArray(m_vao);

        // Activate the zeroth texture slot in OpenGL, and pass the index to the texture uniform in our shader.
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(m_activeShader->getUniformLocation("SpriteTexture"), 0);

        // For each batch, bind its texture, set the sampler state (have to do this each time), and draw the triangles in that batch.
        for (auto& batch : m_batches) {
            glBindTexture(GL_TEXTURE_2D, batch.texture);

            // Note that we pass an offset as the final argument despite glDrawElements expecting a pointer as we have already uploaded
            // the data to the buffer on the GPU - we only need to pass an offset in bytes from the beginning of this buffer rather than
            // the address of a buffer in RAM.
            glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(batch.indexOffset * sizeof(ui32)));
        }

        // Unbind out vertex array.
        glBindVertexArray(0);

        // Deactivate our shader.
        m_activeShader->unuse();
}

void spg::SpriteBatcher::render(const f32m4& worldProjection, const f32v2& screenSize) {
    f32m4 viewProjection = f32m4(
         2.0f / screenSize.x,  0.0f,                0.0f, 0.0f,
         0.0f,                -2.0f / screenSize.y, 0.0f, 0.0f,
         0.0f,                 0.0f,                1.0f, 0.0f,
        -1.0f,                 1.0f,                0.0f, 1.0f
    );

    render(worldProjection, viewProjection);
}

void spg::SpriteBatcher::render(const f32v2& screenSize) {
    f32m4 identity = f32m4(1.0f);

    render(identity, screenSize);
}

void spg::SpriteBatcher::sortSprites(SpriteSortMode sortMode) {
    if (m_spritePtrs.empty()) return;

    // Sort the data according to mode.
    switch (sortMode) {
    case SpriteSortMode::TEXTURE:
        std::stable_sort(m_spritePtrs.begin(), m_spritePtrs.end(), &sortTexture);
        break;
    case SpriteSortMode::FRONT_TO_BACK:
        std::stable_sort(m_spritePtrs.begin(), m_spritePtrs.end(), &sortFrontToBack);
        break;
    case SpriteSortMode::BACK_TO_FRONT:
        std::stable_sort(m_spritePtrs.begin(), m_spritePtrs.end(), &sortBackToFront);
        break;
    default:
        break;
    }
}

void spg::SpriteBatcher::generateBatches() {
    // If we have no sprites, just tell the GPU we have nothing.
    if (m_spritePtrs.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, m_usageHint);
        return;
    }

    // Create a buffer of vertices to be populated and sent to the GPU.
    SpriteVertex* vertices = new SpriteVertex[VERTICES_PER_QUAD * m_spritePtrs.size()];

    // Some counts to help us know where we're at with populating the vertices.
    ui32 vertCount  = 0;
    ui32 indexCount = 0;

    // Create our first batch, which has 0 offset and texture the same as that of
    // the first sprite - as it defines the first batch.
    m_batches.emplace_back();
    auto& batch       = m_batches.back();
    batch.indexOffset = 0;
    batch.texture     = m_sprites[0].texture;

    // For each sprite, we want to populate the vertex buffer with those for that
    // sprite. In the case that we are changing to a new texture, we need to 
    // start a new batch.
    for (auto& sprite : m_spritePtrs) {
        // Start a new batch with texture of the sprite we're currently working with
        // if that texture is different to the previous batch.
        if (sprite->texture != batch.texture) {
            // Now we are making a new batch, we can set the number of indices in 
            // the previous batch.
            batch.indexCount = indexCount - batch.indexOffset;
            m_batches.emplace_back();

            batch = m_batches.back();
            batch.indexOffset = indexCount;
            batch.texture     = sprite->texture;
        }

        // Builds the sprite's quad, i.e. adds the sprite's vertices to the vertex buffer.
        sprite->build(sprite, vertices + vertCount);

        // Update our counts.
        vertCount  += VERTICES_PER_QUAD;
        indexCount += INDICES_PER_QUAD;
    }
    batch.indexCount = indexCount - batch.indexOffset;

    // If we need more indices than we have so far uploaded to the GPU, we must
    // generate more and update the index buffer on the GPU.
    //     For now the index pattern is the same for all sprites, as all sprites are
    //     treated as quads. If we want to support other geometries of sprite we will
    //     have to change this.
    if (m_indexCount < indexCount) {
        m_indexCount = indexCount;

        // Bind the index buffer.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        // Invalidate the old buffer data on the GPU so that when we write our new data we don't
        // need to wait for the old data to be unused by the GPU.
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(ui32), nullptr, m_usageHint);

        // Create a local index buffer we will upload to the GPU.
        ui32* indices = new ui32[m_indexCount];
        ui32 i = 0; // Index cursor.
        ui32 v = 0; // Vertex cursor.
        while (i < m_indexCount) {
            // For each quad, we have four vertices which we write 6 indices for - giving us two triangles.
            // The order of these indices is important - each triple should form a triangle correlating
            // to the build functions.
            indices[i++] = v;     // Top left vertex.
            indices[i++] = v + 2; // Bottom left vertex.
            indices[i++] = v + 3; // Bottom right vertex.
            indices[i++] = v + 3; // Bottom right vertex.
            indices[i++] = v + 1; // Top right vertex.
            indices[i++] = v;     // Top left vertex.

            v += 4;
        }

        // Send the indices over to the GPU.
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_indexCount * sizeof(ui32), indices);
        // Unbind our buffer object.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // Bind the vertex buffer and delete the old data from the GPU.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // Invalidate the old buffer data on the GPU so that when we write our new data we don't
    // need to wait for the old data to be unused by the GPU.
    glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(SpriteVertex), nullptr, m_usageHint);
    // Write our new data.
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertCount * sizeof(SpriteVertex), vertices);
    // Unbind our buffer object.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Clear up memory.
    delete[] vertices;
}

void spg::buildQuad(const Sprite* sprite, SpriteVertex* vertices) {
    SpriteVertex& topLeft    = vertices[0];
    topLeft.position.x       = sprite->position.x;
    topLeft.position.y       = sprite->position.y;
    topLeft.position.z       = sprite->depth;
    topLeft.relativePosition = f32v2(0.0f, 0.0f);
    topLeft.uvDimensions     = sprite->uvDimensions;

    SpriteVertex& topRight    = vertices[1];
    topRight.position.x       = sprite->position.x + sprite->size.x;
    topRight.position.y       = sprite->position.y;
    topRight.position.z       = sprite->depth;
    topRight.relativePosition = f32v2(1.0f, 0.0f);
    topRight.uvDimensions     = sprite->uvDimensions;

    SpriteVertex& bottomLeft    = vertices[2];
    bottomLeft.position.x       = sprite->position.x;
    bottomLeft.position.y       = sprite->position.y + sprite->size.y;
    bottomLeft.position.z       = sprite->depth;
    bottomLeft.relativePosition = f32v2(0.0f, 1.0f);
    bottomLeft.uvDimensions     = sprite->uvDimensions;

    SpriteVertex& bottomRight    = vertices[3];
    bottomRight.position.x       = sprite->position.x + sprite->size.x;
    bottomRight.position.y       = sprite->position.y + sprite->size.y;
    bottomRight.position.z       = sprite->depth;
    bottomRight.relativePosition = f32v2(1.0f, 1.0f);
    bottomRight.uvDimensions     = sprite->uvDimensions;

    switch (sprite->gradient) {
        case Gradient::LEFT_TO_RIGHT:
            topLeft.colour  = bottomLeft.colour  = sprite->c1;
            topRight.colour = bottomRight.colour = sprite->c2;
            break;
        case Gradient::TOP_TO_BOTTOM:
            topLeft.colour    = topRight.colour    = sprite->c1;
            bottomLeft.colour = bottomRight.colour = sprite->c2;
            break;
        case Gradient::TOP_LEFT_TO_BOTTOM_RIGHT:
            topLeft.colour     = sprite->c1;
            bottomRight.colour = sprite->c2;
            topRight.colour    = bottomLeft.colour = lerp(sprite->c1, sprite->c2, 0.5);
            break;
        case Gradient::TOP_RIGHT_TO_BOTTOM_LEFT:
            topRight.colour   = sprite->c1;
            bottomLeft.colour = sprite->c2;
            topLeft.colour    = bottomRight.colour = lerp(sprite->c1, sprite->c2, 0.5);
            break;
        case Gradient::NONE:
            topLeft.colour = topRight.colour = bottomLeft.colour = bottomRight.colour = sprite->c1;
            break;
        default:
            puts("Invalid gradient type!");
            assert(false);
    }
}
