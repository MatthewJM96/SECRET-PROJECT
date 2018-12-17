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
#include "graphics/TextAlign.h"
#include "graphics/WordWrap.hpp"

namespace SecretProject {
    namespace graphics {
        // Forward declarations.
        struct Sprite;
        struct SpriteVertex;

        // The signature of any function capable of building a quad (i.e. a sprite's
        // rectangle).
        using QuadBuilder = void(*)(const Sprite* sprite, SpriteVertex* vertices);

        /**
         * @brief The sorting modes allowed for sorting sprites.
         */
        enum class SpriteSortMode {
            BACK_TO_FRONT,
            FRONT_TO_BACK,
            TEXTURE
        };

        /**
         * @brief The properties that define a sprite.
         */
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

        /**
         * @brief The properties that define a batch - where a
         * batch is a collection of sprites with the same texture
         * that are consecutively positioned within the SpriteBatcher.
         */
        struct SpriteBatch {
            GLuint texture;
            ui32   indexCount;
            ui32   indexOffset;
        };

        /**
         * @brief The properties of a vertex of a sprite. We use this to
         * build up the array of data we need to send to the GPU for rendering.
         */
        struct SpriteVertex {
            f32v3   position;
            f32v2   relativePosition;
            f32v4   uvDimensions;
            colour4 colour;
        };

        /**
         * @brief A set of shader attribute IDs we use for setting and linking
         * variables in our shaders to the data we send to the GPU. (Note how
         * they correspond to the SpriteVertex properties.)
         */
        enum SpriteShaderAttribID : GLuint {
            POSITION = 0,
            RELATIVE_POSITION,
            UV_DIMENSIONS,
            COLOUR,
            SpriteShaderAttribID_SENTINEL
        };

        /**
         * @brief Implementation of sprite batching, sprites are drawn after
         * the sprite batch phase begins, after the end of which they are sorted
         * and their vertex data is collated and sent to the GPU ready for rendering.
         */
        class SpriteBatcher {
            using Sprites    = std::vector<Sprite>;
            using SpritePtrs = std::vector<Sprite*>;
            using Batches    = std::vector<SpriteBatch>;
        public:
            SpriteBatcher();
            ~SpriteBatcher();

            /**
             * @brief Initialises the sprite batcher, setting a font cache, usage
             * hinting, and constructing various default objects (shaders, textures
             * etc.).
             *
             * @param fontCache The font cache to use for obtaining fonts for string
             * drawing.
             * @param usageHint The usage hint we give to OpenGL, telling it how often
             * we expect the sprites to change. Choosing the right value here can
             * improve performance.
             */
            void init(FontCache* fontCache, GLenum usageHint = GL_STATIC_DRAW);
            /**
             * @brief Disposes of the sprite batcher.
             */
            void dispose();

            /**
             * @brief Reserves count many sprite positions in the sprite buffer, if this
             * is less than the number of sprites currently stored it does nothing.
             */
            void reserve(size_t count);

            /**
             * @brief Begins the sprite batching phase. Call this BEFORE ANY call to a
             * "draw" function!
             */
            void begin();

            /**
             * @brief Draw the sprite given.
             *
             * @param The sprite to draw.
             */
            void draw(Sprite&& sprite);
            /**
             * @brief Draw a sprite with the given properties.
             *
             * @param builder The function to use to build a quad.
             * @param texture The texture of the sprite.
             * @param position The position of the sprite.
             * @param size The size of the sprite.
             * @param c1 The first colour of the sprite.
             * @param c2 The second colour of the sprite. Only affects it if a gradient
             * other than Gradient::NONE is selected.
             * @param gradient The gradient of the colours of the sprite.
             * @param depth The depth of the sprite.
             * @param uvRect The normalised UV coordinates and size of the section of
             * the texture given to use for the sprite (i.e. if we want to only use
             * a subsection of the texture).
             */
            void draw( QuadBuilder builder,
                            GLuint texture,
                      const f32v2& position,
                      const f32v2& size,
                           colour4 c1       = { 255, 255, 255, 255 },
                           colour4 c2       = { 255, 255, 255, 255 },
                          Gradient gradient = Gradient::NONE,
                               f32 depth    = 0.0f,
                      const f32v4& uvRect   = f32v4(0.0f, 0.0f, 1.0f, 1.0f));
            /**
             * @brief Draw a sprite with the given properties.
             *
             * @param texture The texture of the sprite.
             * @param position The position of the sprite.
             * @param size The size of the sprite.
             * @param c1 The first colour of the sprite.
             * @param c2 The second colour of the sprite. Only affects it if a gradient
             * other than Gradient::NONE is selected.
             * @param gradient The gradient of the colours of the sprite.
             * @param depth The depth of the sprite.
             * @param uvRect The normalised UV coordinates and size of the section of
             * the texture given to use for the sprite (i.e. if we want to only use
             * a subsection of the texture).
             */
            void draw(      GLuint texture,
                      const f32v2& position,
                      const f32v2& size,
                           colour4 c1       = { 255, 255, 255, 255 },
                           colour4 c2       = { 255, 255, 255, 255 },
                          Gradient gradient = Gradient::NONE,
                               f32 depth    = 0.0f,
                      const f32v4& uvRect   = f32v4(0.0f, 0.0f, 1.0f, 1.0f));

            /**
             * @brief Draw a string with the given properties.
             *
             * Note: this version is only for drawing strings where the
             * entire text has the same properties.
             *
             * @param str The string to draw.
             * @param rect The rectangle in which to draw the string.
             * @param sizing The sizing of the font.
             * @param tint The colour to give the string.
             * @param fontName The name of the font to use.
             * @param fontSize The size of the font to use.
             * @param align The alignment to use for the string.
             * @param wrap The wrapping mode to use for the string.
             * @param depth The depth of the string for rendering.
             * @param style The style of the font to use.
             * @param renderStyle The rendering style to use for the font.
             */
            void drawString(    const char* str,
                                      f32v4 rect,
                               StringSizing sizing,
                                    colour4 tint,
                                const char* fontName,
                                   FontSize fontSize,
                                  TextAlign align       = TextAlign::TOP_LEFT,
                                   WordWrap wrap        = WordWrap::NONE,
                                        f32 depth       = 0.0f,
                                  FontStyle style       = FontStyle::NORMAL,
                            FontRenderStyle renderStyle = FontRenderStyle::BLENDED);
            /**
             * @brief Draw a string with the given properties.
             *
             * Note: this version is only for drawing strings where the
             * entire text has the same properties.
             *
             * @param str The string to draw.
             * @param rect The rectangle in which to draw the string.
             * @param sizing The sizing of the font.
             * @param tint The colour to give the string.
             * @param fontName The name of the font to use.
             * @param align The alignment to use for the string.
             * @param wrap The wrapping mode to use for the string.
             * @param depth The depth of the string for rendering.
             * @param style The style of the font to use.
             * @param renderStyle The rendering style to use for the font.
             */
            void drawString(    const char* str,
                                      f32v4 rect,
                               StringSizing sizing,
                                    colour4 tint,
                                const char* fontName,
                                  TextAlign align       = TextAlign::TOP_LEFT,
                                   WordWrap wrap        = WordWrap::NONE,
                                        f32 depth       = 0.0f,
                                  FontStyle style       = FontStyle::NORMAL,
                            FontRenderStyle renderStyle = FontRenderStyle::BLENDED);
            /**
             * @brief Draw a string with the given properties.
             *
             * Note: this version is only for drawing strings where the
             * entire text has the same properties.
             *
             * @param str The string to draw.
             * @param rect The rectangle in which to draw the string.
             * @param sizing The sizing of the font.
             * @param tint The colour to give the string.
             * @param fontInstnace The instance of the font to use.
             * @param align The alignment to use for the string.
             * @param wrap The wrapping mode to use for the string.
             * @param depth The depth of the string for rendering.
             */
            void drawString( const char* str,
                                   f32v4 rect,
                            StringSizing sizing,
                                 colour4 tint,
                            FontInstance fontInstance,
                               TextAlign align = TextAlign::TOP_LEFT,
                                WordWrap wrap  = WordWrap::NONE,
                                     f32 depth = 0.0f);
            /**
             * @brief Draw a string with the given properties.
             *
             * This version is the most flexible method for drawing strings,
             * enabling multiple components, each its own sub-string possessing
             * its own properties.
             *
             * @param components The components of the string, consisting of
             *                   sub-strings and their properties.
             * @param rect The rectangle in which to draw the string.
             * @param align The alignment to use for the string.
             * @param wrap The wrapping mode to use for the string.
             * @param depth The depth of the string for rendering.
             */
            void drawString(StringComponents components,
                                       f32v4 rect,
                                   TextAlign align = TextAlign::TOP_LEFT,
                                    WordWrap wrap  = WordWrap::NONE,
                                         f32 depth = 0.0f);

            /**
             * @brief Ends the sprite batching phase, the sprites are sorted and
             * the batches are generated, sending the vertex buffers to the GPU. Call
             * this AFTER ALL calls to "draw" functions and BEFORE ANY call to a
             * "render" function.
             */
            void end(SpriteSortMode sortMode = SpriteSortMode::TEXTURE);

            // TODO(Matthew): Do we want to allow a shader per batch? I don't think so,
            //                but unsure.
            /**
             * @brief Sets the shader to be used by the sprite batcher. If the shader
             * that is passed in is unlinked, it is assumed the attributes are to be
             * set as the defaults and so they are set as such and the shader linked.
             *
             * @param shader The shader to use. If this is nullptr, then the default
             * shader is set as the active shader.
             *
             * @return True if the shader was successfully set, false otherwise.
             */
            bool setShader(GLSLProgram* shader = nullptr);

            /**
             * @brief Render the batches that have been generated.
             *
             * @param worldProjection The projection matrix to go from world coords to
             * "camera" coords.
             * @param viewProjection The projection matrix to go from "camera" coords to
             * screen coords.
             */
            void render(const f32m4& worldProjection, const f32m4& viewProjection);
            /**
             * @brief Render the batches that have been generated.
             *
             * This method is useful if you just want to draw to the screen with some
             * sense of the sprites being 2D in the world (e.g. a marker above an NPCs
             * head).
             *
             * @param worldProjection The projection matrix to go from world coords to
             * "camera" coords.
             * @param screenSize The size of the screen.
             */
            void render(const f32m4& worldProjection, const f32v2& screenSize);
            /**
             * @brief Render the batches that have been generated.
             *
             * This method is useful if you just want to draw to the screen without
             * placing the sprites in the world at all (e.g. UI elements like the main
             * menu).
             *
             * @param screenSize The size of the screen.
             */
            void render(const f32v2& screenSize);
        protected:
            /**
             * @brief Sorts the sprites using the given sort mode.
             *
             * @param sortMode The mode by which to sort the sprites.
             */
            void sortSprites(SpriteSortMode sortMode);

            /**
             * @brief Generates batches from the drawn sprites.
             */
            void generateBatches();

            /**
             * @brief Compares sprites by texture ID for sorting (such that all sprites
             * with the same texture will be in one batch together - this is the
             * quickest for rendering by generating the least batches).
             *
             * @param sprite1 The first sprite to compare.
             * @param sprite2 The second sprite to compare.
             */
            static bool sortTexture(Sprite* sprite1, Sprite* sprite2)     { return sprite1->texture < sprite2->texture; }
            /**
             * @brief Compares sprites by depth for sorting (such that sprites
             * with less depth are ordered first).
             *
             * @param sprite1 The first sprite to compare.
             * @param sprite2 The second sprite to compare.
             */
            static bool sortFrontToBack(Sprite* sprite1, Sprite* sprite2) { return sprite1->depth   < sprite2->depth;   }
            /**
             * @brief Compares sprites by depth for sorting (such that sprites
             * with greater depth are ordered first).
             *
             * @param sprite1 The first sprite to compare.
             * @param sprite2 The second sprite to compare.
             */
            static bool sortBackToFront(Sprite* sprite1, Sprite* sprite2) { return sprite1->depth   > sprite2->depth;   }

            std::vector<Sprite>  m_sprites;
            std::vector<Sprite*> m_spritePtrs;

            GLuint m_vao, m_vbo, m_ibo;
            GLenum m_usageHint;
            ui32   m_indexCount;

            ui32        m_defaultTexture;
            GLSLProgram m_defaultShader;

            GLSLProgram* m_activeShader;

            FontCache* m_fontCache;

            std::vector<SpriteBatch> m_batches;
        };

        void buildQuad(const Sprite* sprite, SpriteVertex* vertices);
    }
}
namespace spg = SecretProject::graphics;

#endif // !defined(SP_Graphics_SpriteBatcher_h__)
