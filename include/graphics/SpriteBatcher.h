/**
 * @file SpriteBatcher.h
 * @brief Batches sprites to reduce draw calls needed.
 */

#pragma once

#if !defined(SP_Graphics_SpriteBatcher_h__)
#define SP_Graphics_SpriteBatcher_h__

#include <map>
#include <vector>

#include "types.h"
#include "graphics/Font.h"
#include "graphics/GLSLProgram.h"
#include "graphics/Gradients.hpp"
#include "graphics/TextAlign.hpp"
#include "graphics/WordWrap.hpp"

namespace SecretProject {
    namespace graphics {
        // Forward declarations.
        struct Sprite;
        struct SpriteVertex;

        using QuadBuilder = void(*)(const Sprite* sprite, SpriteVertex* vertices);

        enum class SpriteSortMode {
            BACK_TO_FRONT,
            FRONT_TO_BACK,
            TEXTURE
        };

        struct Sprite {
            QuadBuilder build;
            GLuint      texture;
            f32v2       position;
            f32v2       size;
            f32         depth;
            f32v4       uvDimensions;
            colour4     c1, c2;
            Gradient    gradient;
            // TODO(Matthew): Offsets, rotations, etc?
            // TODO(Matthew): Custom gradients? Different blending styles?
        };

        struct SpriteBatch {
            GLuint texture;
            ui32   indexCount;
            ui32   indexOffset;
        };

        struct SpriteVertex {
            f32v3   position;
            f32v2   relativePosition;
            f32v4   uvDimensions;
            colour4 colour;
        };

        enum SpriteShaderAttribID : GLuint {
            POSITION = 0,
            RELATIVE_POSITION,
            UV_DIMENSIONS,
            COLOUR,
            SpriteShaderAttribID_SENTINEL
        };

        class SpriteBatcher {
            using Sprites    = std::vector<Sprite>;
            using SpritePtrs = std::vector<Sprite*>;
            using Batches    = std::vector<SpriteBatch>;
        public:
            SpriteBatcher();
            ~SpriteBatcher();

            void init(FontCache* fontCache, GLenum usageHint = GL_STATIC_DRAW);
            void dispose();

            void reserve(size_t count);

            void begin();

            void draw(Sprite&& sprite);
            void draw( QuadBuilder builder,
                            GLuint texture,
                      const f32v2& position,
                      const f32v2& size,
                           colour4 c1       = { 255, 255, 255, 255 },
                           colour4 c2       = { 255, 255, 255, 255 },
                          Gradient gradient = Gradient::NONE,
                               f32 depth    = 0.0f,
                      const f32v4& uvRect   = f32v4(0.0f, 0.0f, 1.0f, 1.0f));
            void draw(      GLuint texture,
                      const f32v2& position,
                      const f32v2& size,
                           colour4 c1       = { 255, 255, 255, 255 },
                           colour4 c2       = { 255, 255, 255, 255 },
                          Gradient gradient = Gradient::NONE,
                               f32 depth    = 0.0f,
                      const f32v4& uvRect   = f32v4(0.0f, 0.0f, 1.0f, 1.0f));

            void drawString(    const char* str,
                                      f32v4 rect,
                                      f32v2 scaling,
                                    colour4 tint,
                                const char* fontName,
                                   FontSize fontSize,
                                  TextAlign align       = TextAlign::TOP_LEFT,
                                   WordWrap wrap        = WordWrap::NONE,
                                        f32 depth       = 0.0f,
                                  FontStyle style       = FontStyle::NORMAL,
                            FontRenderStyle renderStyle = FontRenderStyle::BLENDED);
            void drawString(    const char* str,
                                      f32v4 rect,
                                      f32v2 scaling,
                                    colour4 tint,
                                const char* fontName,
                                  TextAlign align       = TextAlign::TOP_LEFT,
                                   WordWrap wrap        = WordWrap::NONE,
                                        f32 depth       = 0.0f,
                                  FontStyle style       = FontStyle::NORMAL,
                            FontRenderStyle renderStyle = FontRenderStyle::BLENDED);
            void drawString( const char* str,
                                   f32v4 rect,
                                   f32v2 scaling,
                                 colour4 tint,
                            FontInstance fontInstance,
                               TextAlign align = TextAlign::TOP_LEFT,
                                WordWrap wrap  = WordWrap::NONE,
                                     f32 depth = 0.0f);

            void end(SpriteSortMode sortMode = SpriteSortMode::TEXTURE);

            // TODO(Matthew): Move specifying custom shader out to being a setter - it needs to be prepared!
            void render(const f32m4& worldProjection, const f32m4& viewProjection, GLSLProgram* shader = nullptr);
            void render(const f32m4& worldProjection, const f32v2& screenSize,     GLSLProgram* shader = nullptr);
            void render(                              const f32v2& screenSize,     GLSLProgram* shader = nullptr);
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

            ui32        m_defaultTexture;
            GLSLProgram m_defaultShader;

            FontCache* m_fontCache;

            std::vector<SpriteBatch> m_batches;
        };

        void buildQuad(const Sprite* sprite, SpriteVertex* vertices);
    }
}
namespace spg = SecretProject::graphics;

#endif // !defined(SP_Graphics_SpriteBatcher_h__)
