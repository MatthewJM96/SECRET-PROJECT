/**
 * @file SpriteBatcher.h
 * @brief Batches sprites to reduce draw calls needed.
 */

#pragma once

#if !defined(SP_Graphics_SpriteBatcher_h__)
#define SP_Graphics_SpriteBatcher_h__

#include "types.h"

namespace SecretProject {
    namespace graphics {
        // Forward declarations.
        class Sprite;
        class SpriteVertex;

        using QuadBuilder = void(*)(const Sprite* sprite, SpriteVertex* vertices);

        enum class SpriteSortMode {
            BACK_TO_FRONT,
            FRONT_TO_BACK,
            TEXTURE
        };

        // TODO(Matthew): Support textures & colours (+ gradients - easing funcs?), allowing different blending styles.
        struct Sprite {
            QuadBuilder build;
            GLuint      texture;
            f32         depth;
        };

        struct SpriteBatch {
            GLuint texture;
            ui32   indexCount;
            ui32   indexOffset;
        };

        struct SpriteVertex {
            f32v3 position;
            f32v4 uvDimensions;
            colour4 colour;
        };

        // TODO(Matthew): Support shader program embedding.
        class SpriteBatcher {
            using Sprites    = std::vector<Sprite>;
            using SpritePtrs = std::vector<Sprite*>;
            using Batches    = std::vector<SpriteBatch>;
        public:
            SpriteBatcher();
            ~SpriteBatcher();

            void init(GLenum usageHint = GL_STATIC_DRAW);
            void dispose();

            void reserve(size_t count);

            void begin();

            void draw(/* ... */);

            void end(SpriteSortMode sortMode = SpriteSortMode::TEXTURE);

            void render();
        protected:
            void sortSprites(SpriteSortMode sortMode);

            void generateBatches();

            static bool sortTexture(Sprite* sprite1, Sprite* sprite2)     { return sprite1->texture < sprite2->texture; }
            static bool sortFrontToBack(Sprite* sprite1, Sprite* sprite2) { return sprite1->depth   < sprite2->depth;   }
            static bool sortBackToFront(Sprite* sprite1, Sprite* sprite2) { return sprite1->depth   > sprite2->depth;   }

            std::vector<Sprite>  m_sprites;
            std::vector<Sprite*> m_spritePtrs;

            GLuint m_vao, m_vbo, m_ibo;
            GLenum m_usageHint;
            ui32   m_indexCount;

            ui32 m_defaultTexture;

            std::vector<SpriteBatch> m_batches;
        };
    }
}
namespace spg = SecretProject::graphics;

#endif // !defined(SP_Graphics_SpriteBatcher_h__)
