//**************************************************************************************************
// File   : ShaderLibrary.cpp
// Author : Cody Melvin
// Date   : 2025/06/13
// Brief  : Implementation file for system to compile and organize shaders
//**************************************************************************************************

#include "ShaderLibrary.h"
#include "Logger.h"

#include <glad/glad.h>

#include <filesystem> 

std::vector<Shader> ShaderLibrary::m_shaders;


Shader ShaderLibrary::GetShader(std::string name)
{
    //Iterate and find shader with the correct name
    for ( Shader s : m_shaders )
    {
        if ( s.Name() == name )
            return s;
    }

    return Shader();
}

Shader ShaderLibrary::GetShader( std::string name, Shader::ShaderType type )
{
    //Iterate and find shader with correct name and type
    for ( Shader s : m_shaders )
    {
        if ( s.Name() == name )
            if ( s.Type() == type )
                return s;
    }

    return Shader();
}

void ShaderLibrary::CompileShaders(std::string directory)
{
    Logger::LogLine( Logger::severityInfo, "Compiling shaders in path {}", directory );

    std::filesystem::path dir(directory);
    if ( !std::filesystem::exists(dir) )
    {
        Logger::LogLine( Logger::severityError, "Shader library failed to open directory {}",
                         directory );
        return;
    }

    for ( const auto& file : std::filesystem::recursive_directory_iterator(dir) )
    {
        Shader shader;
        shader.LoadShader(file);

        if ( !shader.Verify() )
            continue;

        m_shaders.push_back(shader);
    }
}

void ShaderLibrary::DeleteShaders()
{
    for ( Shader s : m_shaders )
        s.Destroy();

    m_shaders.clear();
}