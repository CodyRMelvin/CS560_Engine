//**************************************************************************************************
// File   : Mesh.cpp
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Implementation of class which encapsulates a 3D mesh
//**************************************************************************************************

#include "glm/gtc/type_ptr.hpp"
#define WIRE 0

#include "Mesh.h"

#include <glad/glad.h>

MeshOld::MeshOld( const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices )
    : m_vertices(vertices)
    , m_indices(indices)
{
    InitializeMeshOld();
}

void MeshOld::Destroy()
{
    glDeleteBuffers( 1, &m_VBO );
    glDeleteBuffers( 1, &m_EBO );
    glDeleteVertexArrays( 1, &m_VAO );
    m_VAO = 0;
    m_EBO = 0;
    m_VBO = 0;
}

void MeshOld::Draw( Transform& transform ) const
{
    glBindVertexArray(m_VAO);
#if WIRE
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
#endif
    glUniformMatrix4fv( 0, 1, GL_FALSE, glm::value_ptr( transform.Matrix() ) );
    glUseProgram(0);
    glDrawElements( GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0 );
    glBindVertexArray(0);
}

void MeshOld::InitializeMeshOld()
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
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0 );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nrm) );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex) );

    glBindVertexArray(0);
}

Mesh::Mesh( const Mesh& other )
    : m_primitives(other.m_primitives) 
    , m_drawBuffer(other.m_drawBuffer)
{ }

Mesh& Mesh::operator=( const Mesh& other )
{
    if ( this == &other )
        return *this;

    this->m_drawBuffer = other.m_drawBuffer;
    this->m_primitives = std::vector<Primitive>(other.m_primitives);
    return *this;
}

void Mesh::InitializeVertexArray( size_t index, GLenum primitiveType )
{
    if ( m_primitives.size() <= index )
        m_primitives.resize( index + 1 );

    Primitive& p = m_primitives[index];

    if (p.VAO)
    {
        Logger::LogLine( Logger::severityWarning, "VAO at index {} already exists", index );
        return;
    }

    glCreateVertexArrays( 1, &p.VAO );
    p.primType = primitiveType;

    p.cmd.instanceCount = 1;
    p.cmd.baseInstance = 0;
    p.cmd.baseVertex = 0;
    p.cmd.firstIndex = 0;
}

void Mesh::InitializeVertexBuffer( size_t index, size_t count )
{
    if ( m_primitives.size() <= index )
        m_primitives.resize( index + 1 );

    Primitive& p = m_primitives[index];

    if (p.VBO)
    {
        Logger::LogLine( Logger::severityWarning, "VBO at index {} already exists", index );
        return;
    }


    glCreateBuffers( 1, &p.VBO );
    glNamedBufferData( p.VBO, count * sizeof(Vertex), nullptr, GL_STATIC_DRAW );
}

void Mesh::InitializeIndexBuffer( size_t index, size_t count, GLenum indexType )
{
    if ( m_primitives.size() <= index )
        m_primitives.resize( index + 1 );

    Primitive& p = m_primitives[index];

    if (p.EBO)
    {
        Logger::LogLine( Logger::severityWarning, "EBO at index {} already exists", index );
        return;
    }

    p.indexType = indexType;
    size_t size = indexType == GL_UNSIGNED_SHORT ? sizeof(uint16_t) : sizeof(uint32_t);

    glCreateBuffers( 1, &p.EBO );
    glNamedBufferData( p.EBO, count * size, nullptr, GL_STATIC_DRAW );
    p.cmd.vertexCount = count;
}

void Mesh::InitializeDrawBuffer()
{
    glCreateBuffers( 1, &m_drawBuffer );
    glNamedBufferData( m_drawBuffer, m_primitives.size() * sizeof(Primitive), m_primitives.data(), GL_STATIC_DRAW );
}

void Mesh::Destroy( size_t index )
{
    if ( m_primitives.size() <= index )
        return;

    Primitive& p = m_primitives[index];
    if (p.VAO)
        glDeleteVertexArrays( 1, &p.VAO );
    if (p.VBO)
        glDeleteBuffers( 1, &p.VBO );
    if (p.EBO)
        glDeleteBuffers( 1, &p.EBO );

    m_primitives.erase( m_primitives.begin() + index );
}

void Mesh::DestroyAll()
{
    for ( auto& p : m_primitives )
    {
        if (p.VAO)
            glDeleteVertexArrays( 1, &p.VAO );
        if (p.VBO)
            glDeleteBuffers( 1, &p.VBO );
        if (p.EBO)
            glDeleteBuffers( 1, &p.EBO );
    }

    glDeleteBuffers( 1, &m_drawBuffer );
    m_primitives.clear();
}

void* Mesh::MapVertexBuffer( size_t index, GLenum access )
{
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return nullptr;
    }

    Primitive& p = m_primitives[index];

    if (!p.VBO)
    {
        Logger::LogLine( Logger::severityError, "VBO at index {} does not exist", index );
        return nullptr;
    }

    return glMapNamedBuffer( p.VBO, access );
}

void* Mesh::MapIndexBuffer( size_t index, GLenum access )
{
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return nullptr;
    }

    Primitive& p = m_primitives[index];

    if (!p.EBO)
    {
        Logger::LogLine( Logger::severityError, "EBO at index {} does not exist", index );
        return nullptr;
    }

    return glMapNamedBuffer( p.EBO, access );
}

void Mesh::UnMapVertexBuffer( size_t index )
{
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return;
    }

    Primitive& p = m_primitives[index];

    if (!p.VBO)
    {
        Logger::LogLine( Logger::severityError, "VBO at index {} does not exist", index );
        return;
    }

    glUnmapNamedBuffer(p.VBO);
}

void Mesh::UnMapIndexBuffer( size_t index )
{
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return;
    }

    Primitive& p = m_primitives[index];

    if (!p.EBO)
    {
        Logger::LogLine( Logger::severityError, "EBO at index {} does not exist", index );
        return;
    }

    glUnmapNamedBuffer(p.EBO);
}

void Mesh::EnableVertexAttribute( size_t primIndex, GLuint attribIndex, GLint size, GLenum type, GLuint offset )
{
    if ( m_primitives.size() <= primIndex )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", primIndex );
        return;
    }

    Primitive& p = m_primitives[primIndex];
    glEnableVertexArrayAttrib( p.VAO, attribIndex );
    glVertexArrayAttribFormat( p.VAO, attribIndex, size, type, GL_FALSE, offset );
    glVertexArrayAttribBinding( p.VAO, attribIndex, attribIndex );
    glVertexArrayVertexBuffer( p.VAO, attribIndex, p.VBO, offset, sizeof(Vertex) );
}

void Mesh::EnableIndexBuffer( size_t index )
{
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return;
    }

    Primitive& p = m_primitives[index];
    glVertexArrayElementBuffer( p.VAO, p.EBO );
}

void Mesh::Draw( Transform& t ) const
{
    glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_drawBuffer );
    glUniformMatrix4fv( 0, 1, GL_FALSE, glm::value_ptr( t.Matrix() ));

    for ( int i = 0; i < m_primitives.size(); ++i )
    {
        const Primitive& p = m_primitives[i];
        glBindVertexArray(p.VAO);
        glUseProgram(0);
        glDrawElementsIndirect( p.primType, p.indexType, reinterpret_cast<const void*>( i * sizeof(Primitive) ) );
        glBindVertexArray(0);
    }

}
