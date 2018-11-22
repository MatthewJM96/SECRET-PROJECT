#include "stdafx.h"
#include "graphics/GLSLProgram.h"

#include "io/FileLoader.h"

GLuint spg::GLSLProgram::current = 0;

spg::GLSLProgram::GLSLProgram() :
    m_id(0),
    m_vertexID(0), m_fragID(0),
    m_isLinked(false)
{
    /* Empty */
}

void spg::GLSLProgram::init() {
    if (isInitialised()) return;
    m_id = glCreateProgram();
}

void spg::GLSLProgram::dispose() {
    // Clear the vertex shader if it exists.
    if (m_vertexID != 0) {
        glDeleteShader(m_vertexID);
        m_vertexID = 0;
    }

    // Clear the fragment shader if it exists.
    if (m_fragID != 0) {
        glDeleteShader(m_fragID);
        m_fragID = 0;
    }

    // Clear the shader program if it exists.
    if (m_id != 0) {
        glDeleteProgram(m_id);
        m_id = 0;
        m_isLinked = false;
    }

    // Clear the attribute map.
    ShaderAttributeMap().swap(m_attributes);
}

spg::ShaderCreationResult spg::GLSLProgram::addShader(ShaderInfo shader) {
    // If the program is in an uneditable state, fail.
    if (!isEditable()) {
        return ShaderCreationResult::NON_EDITABLE;
    }

    // Ensure we are targetting a valid shader type, that is not yet built.
    switch (shader.type) {
        case ShaderType::VERTEX:
            if (m_vertexID != 0) return ShaderCreationResult::VERTEX_EXISTS;
            break;
        case ShaderType::FRAGMENT:
            if (m_fragID != 0) return ShaderCreationResult::FRAG_EXISTS;
            break;
        default:
            return ShaderCreationResult::INVALID_STAGE;
    }

    // Create the shader, ready for compilation.
    GLuint shaderID = glCreateShader((GLenum) shader.type);
    if (shaderID == 0) return ShaderCreationResult::CREATE_FAIL;

    // Read in the shader code.
    char* buffer;
    if (!spio::File::read(shader.filepath, buffer)) {
        return ShaderCreationResult::READ_FAIL;
    }

    // Compile our shader code.
    glShaderSource(shaderID, 1, &buffer, nullptr);
    glCompileShader(shaderID);

    // Clear memory.
    delete[] buffer;

    // Check if we succeeded in compilation.
    GLint status = 0;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

        char* log = new char[maxLength + 1];
        glGetShaderInfoLog(shaderID, maxLength, nullptr, log);
        log[maxLength] = '\0';

        // TODO(Matthew): Send out log somehow (event?).

        glDeleteShader(shaderID);
        return ShaderCreationResult::COMPILE_FAIL;
    }

    // Set the appropriate shader ID.
    switch(shader.type) {
        case ShaderType::VERTEX:
            m_vertexID = shaderID;
            break;
        case ShaderType::FRAGMENT:
            m_fragID   = shaderID;
            break;
    }

    return ShaderCreationResult::SUCCESS;
}

spg::ShaderCreationResults spg::GLSLProgram::addShaders(const char* vertexPath, const char* fragmentPath) {
    return ShaderCreationResults{
        addShader(ShaderInfo{ ShaderType::VERTEX,   vertexPath   }),
        addShader(ShaderInfo{ ShaderType::FRAGMENT, fragmentPath })
    };
}

spg::ShaderLinkResult spg::GLSLProgram::link() {
    // If the program is in an uneditable state, fail.
    if (!isEditable()) return ShaderLinkResult::NON_EDITABLE;

    // If we are missing either shader, fail.
    if (!m_vertexID) return ShaderLinkResult::VERTEX_MISSING;
    if (!m_fragID)   return ShaderLinkResult::FRAG_MISSING;

    // Attach our shaders, link program and then detach shaders.
    glAttachShader(m_id, m_vertexID);
    glAttachShader(m_id, m_fragID);

    glLinkProgram(m_id);

    glDetachShader(m_id, m_vertexID);
    glDetachShader(m_id, m_fragID);

    // Clean up our now redundant shaders.
    glDeleteShader(m_vertexID);
    glDeleteShader(m_fragID);
    m_vertexID = 0;
    m_fragID   = 0;

    // Get the result of linking.
    GLint status = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    m_isLinked = (status == GL_TRUE);

    // If we failed to link, get info log and then fail.
    if (!m_isLinked) {
        GLint maxLength = 0;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &maxLength);

        char* log = new char[maxLength + 1];
        glGetProgramInfoLog(m_id, maxLength, nullptr, log);
        log[maxLength] = '\0';

        // TODO(Matthew): Send out log somehow (event?).

        return ShaderLinkResult::LINK_FAIL;
    }

    return ShaderLinkResult::SUCCESS;
}

bool spg::GLSLProgram::setAttribute(const char* name, GLuint index) {
    if (!isEditable()) return false;

    glBindAttribLocation(m_id, index, name);
    m_attributes[name] = index;
}

bool spg::GLSLProgram::setAttributes(const ShaderAttributeMap& attributes) {
    if (!isEditable()) return false;

    for (auto& attribute : attributes) {
        glBindAttribLocation(m_id, attribute.second, attribute.first);
        m_attributes[attribute.first] = attribute.second;
    }
}

GLuint spg::GLSLProgram::getUniformLocation(const char* name) const {
    // Cannot find location of uniform until the program has been linked.
    if (!isLinked()) return GL_INVALID_OPERATION;

    return glGetUniformLocation(m_id, name);
}

void spg::GLSLProgram::enableVertexAttribArrays() const {
    for (auto& attribute : m_attributes) {
        glEnableVertexAttribArray(attribute.second);
    }
}

void spg::GLSLProgram::disableVertexAttribArrays() const {
    for (auto& attribute : m_attributes) {
        glDisableVertexAttribArray(attribute.second);
    }
}

void spg::GLSLProgram::use() {
    if (!isInUse()) {
        glUseProgram(m_id);
        GLSLProgram::current = m_id;
    }
}

void spg::GLSLProgram::unuse() {
    if (GLSLProgram::current != 0) {
        glUseProgram(0);
        GLSLProgram::current = 0;
    }
}
