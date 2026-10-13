//**************************************************************************************************
// File   : Pipeline.cpp
// Author : Cody Melvin
// Date   : 2026/10/13
// Brief  : Implementation of pipeline abstraction class
//**************************************************************************************************

#include "Pipeline.h"
#include "Shader.h"

Pipeline::Pipeline( const Pipeline& other )
    : m_pipeline(other.m_pipeline)
    , m_shaders()
    , m_name(other.m_name)
{
    std::copy( other.m_shaders, other.m_shaders + Shader::shaderTypeMax, m_shaders );
}

Pipeline& Pipeline::operator=( const Pipeline& other )
{ 
    m_pipeline = other.m_pipeline;
    m_name = other.m_name;
    std::copy( other.m_shaders, other.m_shaders + Shader::shaderTypeMax, m_shaders );
    return *this;
}

void Pipeline::Initialize()
{
    glGenProgramPipelines( 1, &m_pipeline );
}

void Pipeline::Destroy()
{
    glDeleteProgramPipelines( 1, &m_pipeline );
}

Pipeline& Pipeline::Shader( const class Shader& shader )
{
    if (shader)
    {
        m_shaders[ shader.Type() ] = shader;
        glUseProgramStages( m_pipeline, shader.Stage(), shader.Program() );
    }

    return *this;
}

class Shader& Pipeline::Shader( const Shader::ShaderType type )
{
    return m_shaders[type];
}

Pipeline& Pipeline::Name( std::string name )
{
    m_name = name;
    return *this;
}

std::string_view Pipeline::Name() const
{
    return m_name;
}

void Pipeline::Bind() const
{
    glBindProgramPipeline(m_pipeline);
}

void Pipeline::UnBind() 
{
    glBindProgramPipeline(0);
}

void Pipeline::UseShader( Shader::ShaderType type ) const
{
    glUseProgram( m_shaders[type].Program() );
}

void Pipeline::UseNoShader() 
{
    glUseProgram(0);
}

void Pipeline::ActiveShader( const Shader::ShaderType type ) const
{
    class Shader s = m_shaders[type];
    if (s)
        glActiveShaderProgram( m_pipeline, s.Program() );
}

bool Pipeline::IsValid() const
{
    glValidateProgramPipeline(m_pipeline);
    GLint check;
    glGetProgramPipelineiv( m_pipeline, GL_VALIDATE_STATUS, &check );

    return check == GL_TRUE;
}

Pipeline::operator bool() const
{
    return IsValid();
}

