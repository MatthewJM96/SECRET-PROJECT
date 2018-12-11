#include "stdafx.h"
#include "graphics/SpriteBatcher.h"

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

void spg::SpriteBatcher::init(GLenum usageHint /*= GL_STATIC_DRAW*/) {
    m_usageHint = usageHint;

    /******************\
     * Create the VAO *
    \******************/

    // Gen the vertex array object and bind it.
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Generate the associated vertex & index buffers.
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    // Bind those buffers.
    glBindBuffer(GL_ARRAY_BUFFER,         m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    // TODO(Matthew): Establish vertex attribute pointers for shader program.

    // Clean everything up.
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

    /*****************************\
     * Create a default shader . *
    \*****************************/

    // Create a default shader.
    m_defaultShader.init();
    // TODO(Matthew): Handle errors.
    m_defaultShader.addShaders("shaders/DefaultSprite.vert", "shaders/DefaultSprite.frag");
    m_defaultShader.link();
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
                                       f32 depth,
                              const f32v4& uvRect /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/) {
    m_sprites.emplace_back(Sprite{
        builder,
        texture == 0 ? m_defaultTexture : texture,
        position,
        size,
        depth,
        uvRect,
        colour4{ 128, 128, 0, 255 },
        colour4{ 128, 128, 0, 255 },
        Gradient::NONE
    });
}

void spg::SpriteBatcher::draw(      GLuint texture,
                              const f32v2& position,
                              const f32v2& size,
                                       f32 depth,
                              const f32v4& uvRect /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/) {
    m_sprites.emplace_back(Sprite{
        &buildQuad,
        texture == 0 ? m_defaultTexture : texture,
        position,
        size,
        depth,
        uvRect,
        colour4{ 128, 128, 0, 255 },
        colour4{ 128, 128, 0, 255 },
        Gradient::NONE
    });
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

void spg::SpriteBatcher::render(const f32m4& worldProjection, const f32m4& viewProjection,
                                    GLSLProgram* shader /*= nullptr*/) {
        // Use the default shader if one isn't provided.
        if (shader == nullptr) shader = &m_defaultShader;

        // Activate the shader.
        shader->use();

        // Upload our projection matrices.
        glUniformMatrix4fv(shader->getUniformLocation("WorldProjection"), 1, false, &worldProjection[0][0]);
        glUniformMatrix4fv(shader->getUniformLocation("ViewProjection"),  1, false, &viewProjection[0][0]);

        // Bind our vertex array.
        glBindVertexArray(m_vao);

        // Activate the zeroth texture slot in OpenGL, and pass the index to the texture uniform in our shader.
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(shader->getUniformLocation("SpriteTexture"), 0);

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
        shader->unuse();
}

void spg::SpriteBatcher::render(const f32m4& worldProjection, const f32v2& screenSize,
                                    GLSLProgram* shader /*= nullptr*/) {
    f32m4 viewProjection = f32m4(
         2.0f / screenSize.x,  0.0f,                0.0f, 0.0f,
         0.0f,                -2.0f / screenSize.y, 0.0f, 0.0f,
         0.0f,                 0.0f,                1.0f, 0.0f,
        -1.0f,                 1.0f,                0.0f, 1.0f
    );

    render(worldProjection, viewProjection, shader);
}

void spg::SpriteBatcher::render(const f32v2& screenSize, GLSLProgram* shader /*= nullptr*/) {
    f32m4 identity = f32m4(1.0f);

    render(identity, screenSize, shader);
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
