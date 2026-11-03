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
        // Gettter for shader based on name solely
        static Shader GetShader(std::string name);
        // Getter for shader based on name and tyoe
        static Shader GetShader( std::string name, Shader::ShaderType type );
        // Compile all shaders in a directory
        static void CompileShaders(std::string directory);
        // Delete all shaders
        static void DeleteShaders();

    private:
        static std::vector<Shader> m_shaders;        
};

#endif