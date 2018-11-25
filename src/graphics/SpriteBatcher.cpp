#include "stdafx.h"
#include "graphics/SpriteBatcher.h"

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

    // TODO(Matthew): establish vertex attribute pointers for shader program.

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

void spg::SpriteBatcher::draw(/* ... */) {

}

void spg::SpriteBatcher::end(SpriteSortMode sortMode /*= SpriteSortMode::TEXTURE*/) {
    // Reserve the right amount of space to then assign a pointer for each sprite.
    if (m_spritePtrs.capacity() < m_sprites.size()) {
        m_spritePtrs.reserve(m_sprites.size());
    }
    for (size_t i = 0; i < m_sprites.size(); ++i) {
        m_spritePtrs[i] = &m_sprites[i];
    }
    
    // Sort the sprites - we sort the vector of pointers only for speed.
    sortSprites(sortMode);

    // Generate the batches to use for draw calls.
    generateBatches();
}

void spg::SpriteBatcher::render() {
    
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
    if (m_spritePtrs.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, m_usageHint);
        return;
    }

    // TODO(Matthew): Complete...
}
