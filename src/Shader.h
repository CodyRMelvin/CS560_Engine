//**************************************************************************************************
// File   : Shader.h
// Author : Cody Melvin
// Date   : 2026/09/18
// Brief  : Interface for shader/program abstraction object 
//**************************************************************************************************

#pragma once
#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"

#include <string>
#include <filesystem>

class Shader
{
    public:
        enum ShaderType
        {
            shaderTypeInvalid = -1,
            shaderTypeVertex,
            shaderTypeFragment,
            shaderTypeCompute,
            shaderTypeGeometry,
            shaderTypeMax,
        };

        Shader();
        Shader( const Shader& other );
        ~Shader();

        Shader& operator=( const Shader& other );

        void LoadShader(std::filesystem::path path);
        void LoadShader(std::string path);
        void Destroy();

        bool Verify() const;
        bool IsValid() const;
        operator bool() const;

        std::string_view Name() const {return m_name;}
        ShaderType Type() const {return m_type;}
        GLenum GLenumType() const;
        GLuint Program() const {return m_program;} 
        GLbitfield Stage() const;

    private:
        GLuint m_shader;
        GLuint m_program;
        std::string m_name;
        ShaderType m_type;
};

#endif
