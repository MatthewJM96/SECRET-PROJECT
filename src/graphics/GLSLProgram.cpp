#include "stdafx.h"
#include "graphics/GLSLProgram.h"

GLuint spgraphics::GLSLProgram::current = 0;

void spgraphics::GLSLProgram::init() {
    if (isInitialised()) return;
    m_id = glCreateProgram();
}

void spgraphics::GLSLProgram::dispose() {
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

