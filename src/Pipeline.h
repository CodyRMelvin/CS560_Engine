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
        // Rule of three
        Pipeline() = default;
        Pipeline( const Pipeline& other );
        Pipeline& operator=( const Pipeline& other );
        ~Pipeline() = default;

        // Create the pipeline object
        void Initialize();
        // Destroy the pipeline object
        void Destroy();

        // Getter for the opengl pipeline object
        GLuint GLPipeline() { return m_pipeline; }
        // Getters and setters for shaders in the pipeline
        Pipeline& Shader( const class Shader& shader );
        class Shader& Shader( const Shader::ShaderType type );
        // Getters and setters for the pipeline name
        Pipeline& Name( std::string name );
        std::string_view Name() const;

        // Bind the pipeline to the opengl state machine
        void Bind() const;
        // Static method to unbind any pipelines
        static void UnBind();
        // Tell the opengl state machine to use the shader of the given type
        void UseShader( Shader::ShaderType type ) const;
        // Static method to tell opengl state machine to not use any shader
        static void UseNoShader();
        // Tell the opengl state machine to set the shader of the given type as active
        void ActiveShader( const Shader::ShaderType type ) const;

        // Methods for passing in uniforms to the pipeline
        void UniformMat( int location, const glm::mat4& uniform ) const;
        void UniformMat( int location, glm::mat4* mats, size_t size ) const;
        template <size_t S> void UniformVec( int location, glm::vec< S, float > uniform  ) const;
        void UniformFloat( int location, float uniform  ) const;
        void UniformInt( int location, int uniform  ) const;

        // Check if the pipeline is valid via either method of boolean operator
        bool IsValid() const;
        operator bool() const;

    private:
        GLuint m_pipeline;
        class Shader m_shaders[Shader::shaderTypeMax];
        std::string m_name;
};

#endif
