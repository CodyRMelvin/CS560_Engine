//**************************************************************************************************
// File   : Pipeline.h
// Author : Cody Melvin
// Date   : 2026/10/13
// Brief  : Interface of class which abstract opengl pipeline object 
//**************************************************************************************************

#pragma once
#ifndef PIPELINE_H
#define PIPELINE_H

#include "Common.h"
#include "Shader.h"

#include "glad/glad.h"

class Pipeline
{
    public:
        Pipeline() = default;
        Pipeline( const Pipeline& other );
        ~Pipeline() = default;

        Pipeline& operator=( const Pipeline& other );

        void Initialize();
        void Destroy();

        GLuint GLPipeline() { return m_pipeline; }
        Pipeline& Shader( const class Shader& shader );
        class Shader& Shader( const Shader::ShaderType type );
        Pipeline& Name( std::string name );
        std::string_view Name() const;

        void Bind() const;
        static void UnBind();
        void UseShader( Shader::ShaderType type ) const;
        static void UseNoShader();
        void ActiveShader( const Shader::ShaderType type ) const;

        bool IsValid() const;
        operator bool() const;

    private:
        GLuint m_pipeline;
        class Shader m_shaders[Shader::shaderTypeMax];
        std::string m_name;
};

#endif
