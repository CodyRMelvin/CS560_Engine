//**************************************************************************************************
// File   : ShaderLibrary.h
// Author : Cody Melvin
// Date   : 2025/06/13
// Brief  : Header file for system to compile and organize shaders
//**************************************************************************************************

#pragma once
#ifndef SHADERLIBRARY_H
#define SHADERLIBRARY_H

#include <glad/glad.h>

#include <vector>
#include <string>

class Shader
{
    public:
        enum ShaderType
        {
            shaderTypeFragment,
            shaderTypeVertex,
            shaderTypeCompute,
            shaderTypeGeometry
        };

        Shader( GLuint address, std::string name, ShaderType type );

        const GLuint m_shaderAddress;
        const std::string m_shaderName;
        const ShaderType m_type;
};


class ShaderLibrary
{
    public:
        static GLuint GetShaderAddress(std::string name);
        static GLuint GetShaderAddress( std::string name, Shader::ShaderType type );
        static void CompileShaders(std::string directory);
        static void DeleteShaders();

    private:
        static std::vector<Shader> m_shaders;        
};

#endif