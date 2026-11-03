//**************************************************************************************************
// File   : Mesh.cpp
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Implementation of class which encapsulates a 3D mesh
//**************************************************************************************************

#include "Mesh.h"
#include "RenderAnimatedIndirectCMD.h"
#include "RenderAnimationBonesCMD.h"
#include "ShaderLibrary.h"
#include "RenderDevice.h"
#include "RenderIndirectCMD.h"  

#include <glad/glad.h>

Mesh::Mesh( uint32_t id, uint32_t parent )
    : Node3D( id, parent )
    , m_primitives()
    , m_drawBuffer()
    , m_skin()
    , m_animationPrimitiveIndex()
{ }

Mesh::Mesh( const Mesh& other )
    : Node3D(other)
    , m_primitives(other.m_primitives) 
    , m_drawBuffer(other.m_drawBuffer)
    , m_skin(other.m_skin)
    , m_animationPrimitiveIndex(other.m_animationPrimitiveIndex)
{ }

Mesh& Mesh::operator=( const Mesh& other )
{
    if ( this == &other )
        return *this;

    // copy over data from from parent class
    *static_cast<Node3D*>(this) = other;

    m_drawBuffer = other.m_drawBuffer;
    m_primitives = std::vector<Primitive>(other.m_primitives);
    m_skin = other.m_skin;
    m_animationPrimitiveIndex = other.m_animationPrimitiveIndex;
    return *this;
}

void Mesh::Initialize()
{
    // Create the draw buffer
    InitializeDrawBuffer();
    // Instantiate a default pipeline using the FlatShade shaders
    // TODO: Replace with pipeline library
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
    RenderDeviceSettings config = RenderDevice::Get().config;

    // If the mesh has a skin and we are rendering meshes as skinned, create a
    // RenderAnimatedIndirectCMD render command and pass it to the render device
    if ( m_skin.has_value() && config.renderSkinned )
    {
        // Temporary implementation
        // TODO: implement pipeline library
        static Pipeline p;
        if ( p.GLPipeline() == 0 ) 
        {
            p.Initialize();
            p.Shader( ShaderLibrary::GetShader( "Animation", Shader::shaderTypeVertex) )
             .Shader( ShaderLibrary::GetShader( "FlatShade", Shader::shaderTypeFragment ) );
            p.Bind();
        }

        RenderAnimatedIndirectCMD* cmd = new RenderAnimatedIndirectCMD();
        cmd->meshID = m_ID;
        cmd->pipeline = p;
        RenderDevice::Get().Push(cmd);
    }
    // Otherwise pass a RenderIndirectCMD render command
    else
    {
        RenderIndirectCMD* cmd = new RenderIndirectCMD();
        cmd->mesh = m_ID;
        cmd->transform = GlobalTransformMatrix();
        cmd->pipeline = m_pipeline;

        RenderDevice::Get().Push(cmd); 
    }
    // If this is a skinned mesh and skeleton rendering is enabled
    // pass a RenderAnimationBonesCMD render command to the render device
    if ( config.renderSkeleton && m_skin.has_value() )
    {
        RenderAnimationBonesCMD* cmd = new RenderAnimationBonesCMD();
        cmd->meshIndex = m_ID;
        cmd->transform = GlobalTransformMatrix();
        cmd->pipeline = m_pipeline;

        RenderDevice::Get().Push( cmd, 1 ); 
    }
}

void Mesh::Destroy()
{
    // Delete the VAO and buffers for each primitive
    for ( auto& p : m_primitives )
    {
        if (p.VAO)
            glDeleteVertexArrays( 1, &p.VAO );
        if (p.VBO)
            glDeleteBuffers( 1, &p.VBO );
        if (p.EBO)
            glDeleteBuffers( 1, &p.EBO );
    }

    // Delete the draw buffer and clear primitive data
    glDeleteBuffers( 1, &m_drawBuffer );
    m_primitives.clear();
}

void Mesh::InitializeVertexArray( size_t index, GLenum primitiveType )
{
    // Force primitive vector to contain a primitive at the given index
    if ( m_primitives.size() <= index )
        m_primitives.resize( index + 1 );

    Primitive& p = m_primitives[index];

    // Ensure VAO is not already created
    if (p.VAO)
    {
        Logger::LogLine( Logger::severityWarning, "VAO at index {} already exists", index );
        return;
    }

    // Create the VAO
    glCreateVertexArrays( 1, &p.VAO );
    p.primType = primitiveType;

    // Default parameters for indirect drawing
    p.cmd.instanceCount = 1;
    p.cmd.baseInstance = 0;
    p.cmd.baseVertex = 0;
    p.cmd.firstIndex = 0;
}

void Mesh::InitializeVertexBuffer( size_t index, size_t count, size_t vertexSize )
{
    // Force primitive vector to contain a primitive at the given index
    if ( m_primitives.size() <= index )
        m_primitives.resize( index + 1 );

    Primitive& p = m_primitives[index];

    // Ensure the VBO does not alreadu exist
    if (p.VBO)
    {
        Logger::LogLine( Logger::severityWarning, "VBO at index {} already exists", index );
        return;
    }

    // Create the VBO
    glCreateBuffers( 1, &p.VBO );
    glNamedBufferData( p.VBO, count * vertexSize, nullptr, GL_STATIC_DRAW );
}

void Mesh::InitializeIndexBuffer( size_t index, size_t count, GLenum indexType )
{
    // Force primitive vector to contain a primitive at the given index
    if ( m_primitives.size() <= index )
        m_primitives.resize( index + 1 );

    Primitive& p = m_primitives[index];

    // Ensure the index buffer does not already exist
    if (p.EBO)
    {
        Logger::LogLine( Logger::severityWarning, "EBO at index {} already exists", index );
        return;
    }

    // Get index size based on passed in enum value
    p.indexType = indexType;
    size_t size = indexType == GL_UNSIGNED_SHORT ? sizeof(uint16_t) : sizeof(uint32_t);

    // Create the index buffer
    glCreateBuffers( 1, &p.EBO );
    glNamedBufferData( p.EBO, count * size, nullptr, GL_STATIC_DRAW );
    p.cmd.vertexCount = count;
}

void Mesh::InitializeDrawBuffer()
{
    // Create the draw buffer
    glCreateBuffers( 1, &m_drawBuffer );
    glNamedBufferData( m_drawBuffer, m_primitives.size() * sizeof(Primitive), m_primitives.data(), GL_STATIC_DRAW );
}

size_t Mesh::GetAnimationPrimitive()
{
    // Return the index of the animation primitive if it exists
    if ( m_animationPrimitiveIndex.has_value() ) 
        return m_animationPrimitiveIndex.value();

    // Create a blank primitive and return it's index if one does not already exist
    m_animationPrimitiveIndex = m_primitives.size();
    m_primitives.push_back( Primitive{ IndirectDrawCommand{0}, 0 } );
    return m_animationPrimitiveIndex.value();
}

void Mesh::Destroy( size_t index )
{
    // Safety check
    if ( m_primitives.size() <= index )
        return;

    // Delete VAO and buffers in primitive of given index
    Primitive& p = m_primitives[index];
    if (p.VAO)
        glDeleteVertexArrays( 1, &p.VAO );
    if (p.VBO)
        glDeleteBuffers( 1, &p.VBO );
    if (p.EBO)
        glDeleteBuffers( 1, &p.EBO );

    // Erase primitive at given index
    m_primitives.erase( m_primitives.begin() + index );
}

void* Mesh::MapVertexBuffer( size_t index, GLenum access )
{
    // Safety check
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return nullptr;
    }

    Primitive& p = m_primitives[index];

    // Ensure vertex buffer exists
    if (!p.VBO)
    {
        Logger::LogLine( Logger::severityError, "VBO at index {} does not exist", index );
        return nullptr;
    }

    // Map the vertex buffer to cpu memory and return it's address
    return glMapNamedBuffer( p.VBO, access );
}

void* Mesh::MapIndexBuffer( size_t index, GLenum access )
{
    // Safety check
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return nullptr;
    }

    Primitive& p = m_primitives[index];

    // Ensure index buffer exists
    if (!p.EBO)
    {
        Logger::LogLine( Logger::severityError, "EBO at index {} does not exist", index );
        return nullptr;
    }

    // Map the index buffer to cpu memory and return it's address
    return glMapNamedBuffer( p.EBO, access );
}

void Mesh::UnMapVertexBuffer( size_t index )
{
    // Safety check
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return;
    }

    Primitive& p = m_primitives[index];

    // Ensure vertex buffer exists
    if (!p.VBO)
    {
        Logger::LogLine( Logger::severityError, "VBO at index {} does not exist", index );
        return;
    }

    // Return cpu representation of vertex buffer to the gpu
    glUnmapNamedBuffer(p.VBO);
}

void Mesh::UnMapIndexBuffer( size_t index )
{
    // Safety check
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return;
    }

    Primitive& p = m_primitives[index];

    // Endure index buffer exists
    if (!p.EBO)
    {
        Logger::LogLine( Logger::severityError, "EBO at index {} does not exist", index );
        return;
    }

    // Return cpu representation of index buffer to gpu
    glUnmapNamedBuffer(p.EBO);
}

void Mesh::EnableVertexAttribute( size_t primIndex, GLuint attribIndex, GLint size, GLenum type, GLuint offset, size_t vertexSize )
{
    // Safety check
    if ( m_primitives.size() <= primIndex )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", primIndex );
        return;
    }

    // Enable a vertex float attribute at the given primitive with the given information
    Primitive& p = m_primitives[primIndex];
    glEnableVertexArrayAttrib( p.VAO, attribIndex );
    glVertexArrayAttribFormat( p.VAO, attribIndex, size, type, GL_FALSE, 0 );
    glVertexArrayAttribBinding( p.VAO, attribIndex, attribIndex );
    glVertexArrayVertexBuffer( p.VAO, attribIndex, p.VBO, offset, vertexSize );
}

void Mesh::EnableVertexAttributeI( size_t primIndex, GLuint attribIndex, GLint size, GLenum type, GLuint offset, size_t vertexSize )
{
    // Safety check
    if ( m_primitives.size() <= primIndex )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", primIndex );
        return;
    }

    // Enable an integer vertex attribute at the given primitive with the given information
    Primitive& p = m_primitives[primIndex];
    glEnableVertexArrayAttrib( p.VAO, attribIndex );
    glVertexArrayAttribIFormat( p.VAO, attribIndex, size, type, 0 );
    glVertexArrayAttribBinding( p.VAO, attribIndex, attribIndex );
    glVertexArrayVertexBuffer( p.VAO, attribIndex, p.VBO, offset, vertexSize );
}

void Mesh::EnableIndexBuffer( size_t index )
{
    // Safety check
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return;
    }

    // bind the index buffer to the vertex array
    Primitive& p = m_primitives[index];
    glVertexArrayElementBuffer( p.VAO, p.EBO );
}

void Mesh::DrawPrimitives() const
{
    // Do not draw if we are not rendering meshes
    RenderDeviceSettings& config = RenderDevice::Get().config;
    if (!config.renderMesh)
        return;

    // If there is a primitive for drawing the skeleton, perform the draw operation while checking against the index 
    // to ensure the skeleton primitive isn't drawn here.
    m_animationPrimitiveIndex.and_then(
    [ &config, this ]( const size_t idx ) -> std::optional<size_t>
    {
        glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_drawBuffer );
        for ( int i = 0; i < m_primitives.size(); ++i )
        {
            if ( idx == i )
                continue;

            DrawPrimitive(i);
        }
        return idx;
    })
    // If there is no skeleton primitive, iterate through and draw without concern
    .or_else(
    [ &config, this ]() -> std::optional<size_t>
    {
        glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_drawBuffer );
        for ( int i = 0; i < m_primitives.size(); ++i )
            DrawPrimitive(i);

        return false;
    });
}

void Mesh::DrawAnimationPrimitive() const
{
    if (!RenderDevice::Get().config.renderSkeleton)
        return;

    // Draw the skeleton primitive is there is a supplied index
    m_animationPrimitiveIndex.and_then(
    [this]( const size_t idx ) -> std::optional<size_t>
    {
        glBindBuffer( GL_DRAW_INDIRECT_BUFFER, m_drawBuffer );
        DrawPrimitive(idx);
        return idx;
    })
    // Otherwise, log a warning
    .or_else(
    [this]() -> std::optional<size_t>
    { 
        Logger::LogLine( Logger::severityWarning, "Mesh {} does not have a skeleton primitive index", m_ID ); 
        return false;
    });
}

void Mesh::DrawPrimitive( size_t index ) const
{
    // Safety check
    if ( m_primitives.size() <= index )
    {
        Logger::LogLine( Logger::severityError, "Primitive at index {} does not exist", index );
        return;
    }

    // Perform an indirect draw using the data at the given primitive
    const Primitive& p = m_primitives[index];
    glBindVertexArray(p.VAO);
    glDrawElementsIndirect( p.primType, p.indexType, reinterpret_cast<const void*>( index * sizeof(Primitive) ) );
    glBindVertexArray(0);
}
