//**************************************************************************************************
// File   : ShaderLibrary.h
// Author : Cody Melvin
// Date   : 2025/06/13
// Brief  : Header file for system to compile and organize shaders
//**************************************************************************************************

#pragma once
#ifndef SHADERLIBRARY_H
#define SHADERLIBRARY_H

#include "Shader.h"

#include <glad/glad.h>

#include <vector>
#include <string>

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