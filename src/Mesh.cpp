//**************************************************************************************************
// File   : Mesh.cpp
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Implementation of class which encapsulates a 3D mesh
//**************************************************************************************************

#include "Mesh.h"
#include "ShaderLibrary.h"
#include "RenderDevice.h"
#include "RenderIndirectCMD.h"  

#include <glad/glad.h>

Mesh::Mesh( uint32_t id, uint32_t parent )
    : Node3D( id, parent )
    , m_primitives()
    , m_drawBuffer()
{ }

Mesh::Mesh( const Mesh& other )
    : Node3D(other)
    , m_primitives(other.m_primitives) 
    , m_drawBuffer(other.m_drawBuffer)
{ }

Mesh& Mesh::operator=( const Mesh& other )
{
    if ( this == &other )
        return *this;

    *static_cast<Node3D*>(this) = other;

    this->m_drawBuffer = other.m_drawBuffer;
    this->m_primitives = std::vector<Primitive>(other.m_primitives);
    return *this;
}

void Mesh::Initialize()
{
    m_pipeline.Initialize();
    m_pipeline.Shader( ShaderLibrary::GetShader( "FlatShade", Shader::shaderTypeVertex ) )
              .Shader( ShaderLibrary::GetShader( "FlatShade", Shader::shaderTypeFragment ) );

    m_pipeline.Bind();
    if(!m_pipeline)
    {

        GLint logLength;
        glGetProgramPipelineiv(m_pipeline.GLPipeline(), GL_INFO_LOG_LENGTH, &logLength);
        if ( logLength > 0 )
        {
            std::vector<char> log(logLength);
            glGetProgramPipelineInfoLog(m_pipeline.GLPipeline(), logLength, nullptr, log.data());    
            std::runtime_error e( std::format( "Failed to construct m_pipeline: {}", log.data() ) );
            throw(e);
        }

        std::runtime_error e("Failed to construct m_pipeline");
        throw(e);
    }
}

void Mesh::Render()
{
    RenderIndirectCMD* cmd = new RenderIndirectCMD();
    cmd->mesh = m_ID;
    cmd->transform = m_transform;
    cmd->pipeline = m_pipeline;

    RenderDevice::Get().Push(cmd); 
}

void Mesh::Destroy()
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

void Mesh::DrawPrimitives() const
{
    glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_drawBuffer );
    for ( int i = 0; i < m_primitives.size(); ++i )
    {
        const Primitive& p = m_primitives[i];
        glBindVertexArray(p.VAO);
        glDrawElementsIndirect( p.primType, p.indexType, reinterpret_cast<const void*>( i * sizeof(Primitive) ) );
        glBindVertexArray(0);
    }
}
