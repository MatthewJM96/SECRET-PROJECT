/**
 * @file GLSLProgram.h
 * @brief Provides a wrapper around a specific OpenGL shader program, providing attribute setting and linking.
 */

#pragma once

#if !defined(SP_Graphics_GLSLPogram_h__)
#define SP_Graphics_GLSLPogram_h__

#include "types.h"

namespace SecretProject {
    namespace graphics {
        /**
         * @brief Enumerates the types of shader.
         */
        enum class ShaderType {
            FRAGMENT = GL_FRAGMENT_SHADER,
            VERTEX   = GL_VERTEX_SHADER
        };

        /**
         * @brief Information needed to compile a shader.
         */
        struct ShaderInfo {
            ShaderType  type;
            const char* filepath;
        };

        using ShaderAttributeMap = std::map<const char*, GLuint>;

        class GLSLProgram {
        public:
            GLSLProgram()  { /* Empty */ }
            ~GLSLProgram() { /* Empty */ }

            /**
             * @brief Initialises a shader program.
             */
            void init();
            /**
             * @brief Disposes of a shader program.
             */
            void dispose();

            /**
             * @brief Returns the ID of the shader program.
             *
             * @return The ID of the shader program.
             */
            GLuint getID() const { return m_id; }

            bool isInitialised() const { return m_id != 0;                      }
            bool isLinked()      const { return m_isLinked;                     }
            bool isEditable()    const { return !isLinked() && isInitialised(); }
            bool isInUse()       const { return m_id == GLSLProgram::current;   }

            /**
             * @brief Adds a shader to the program.
             *
             * @param shader The information regarding the shader to be added.
             *
             * @return True if the shader is successfully added, false otherwise.
             */
            bool addShader(ShaderInfo shader);
            /**
             * @brief Adds both a vertex and a fragment shader to the program.
             *
             * @param vertexPath The filepath of the vertex shader to be added.
             * @param fragmentPath The filepath of the fragment shader to be added.
             *
             * @return True if the shaders are successfully added, false otherwise.
             */
            bool addShaders(const char* vertexPath, const char* fragmentPath);

            /**
             * @brief Links the shaders to the shader program.
             *
             * @return True if the shaders are successfully linked, false otherwise.
             */
            bool link();

            /**
             * @brief Sets an attribute with the given name to the given index.
             */
            void setAttribute(const char* name, GLuint index);
            /**
             * @brief Sets a set of attributes.
             */
            void setAttributes(const ShaderAttributeMap& attributes);

            GLuint getAttributeLocation(const char* name) const { return m_attributes.at(name); }
            GLuint getUniformLocation(const char* name)   const;

            static GLuint current;
        protected:

            GLuint m_id;
            GLuint m_vertexID, m_fragID;
            bool   m_isLinked;

            ShaderAttributeMap m_attributes;
        };
    }
}
namespace spgraphics = SecretProject::graphics;

#endif // !defined(SP_Graphics_GLSLPogram_h__)
