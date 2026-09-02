//**************************************************************************************************
// File   : Mesh.cpp
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Implementation of class which encapsulates a 3D mesh
//**************************************************************************************************

#include "Mesh.h"

#include <glad/glad.h>

Mesh::Mesh( const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices )
    : m_vertices(vertices)
    , m_indices(indices)
{
    InitializeMesh();
}

void Mesh::InitializeMesh()
{
    glCreateVertexArrays( 1, &m_VAO );
    glBindVertexArray(m_VAO);

    //Allocate a buffer and bind it to the vao as a vertex buffer
    glCreateBuffers( 1, &m_VBO);
    glBindBuffer( GL_ARRAY_BUFFER, m_VBO);
    //Copy data to the bound buffer
    //GL_STATIC_DRAW states that the data will be copied once and used multiple times as source- 
    //for drawing and image specification commands
    glBufferData( GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW );

    // Same as prior, but for element buffer
    glCreateBuffers( 1, &m_EBO);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), &m_indices[0], GL_STATIC_DRAW );

    glEnableVertexAttribArray(0);
    //Defines information for the currently bound buffer, such as it's index, size, data type, and stride
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0 );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)offsetof(Vertex, nrm) );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)offsetof(Vertex, tex) );

    glBindVertexArray(0)
}

