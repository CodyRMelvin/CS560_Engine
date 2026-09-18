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
#include <vector>

class Shader
{
    public:
        enum ShaderType
        {
            shaderTypeFragment,
            shaderTypeVertex,
            shaderTypeCompute,
            shaderTypeGeometry,
            shaderTypeTesselation
        };

        Shader();
        ~Shader();

    private:
        struct ShaderElement
        {
            GLuint m_shaderAddress;
            std::string m_shaderName;
            ShaderType m_types;
        };
        
};

#endif
