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
#include <fstream>

std::vector<Shader> ShaderLibrary::m_shaders;

bool VerifyShader(GLuint shader)
{
    //Check the compile status of the shader
    GLint compStatus = 0;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &compStatus );

    if (compStatus == GL_TRUE)
        return true;

    //Get length of log string
    GLint size = 0;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &size );

    if ( size == 0 )
    {
        Logger::LogLine( Logger::severityWarning, "Shader compilation failed, unknown reason" );
        return false;
    }
        
    //Preallocate a character vector of size length
    char * errorMSG = new char[size];

    //Write log to vector string
    glGetShaderInfoLog( shader, size, &size, errorMSG );

    //Print and return
    Logger::LogLine( Logger::severityWarning, "Shader compilation failed: {}", errorMSG );

    delete[] errorMSG;
    return false;
}

Shader::Shader( GLuint address, std::string name, ShaderType type )
    : m_shaderAddress(address)
    , m_shaderName(name)
    , m_type(type)
{ }

GLuint ShaderLibrary::GetShaderAddress(std::string name)
{
    //Iterate and find shader with the correct name
    for ( Shader s : m_shaders )
    {
        if ( s.m_shaderName == name )
            return s.m_shaderAddress;
    }

    return 0;
}

GLuint ShaderLibrary::GetShaderAddress( std::string name, Shader::ShaderType type )
{
    //Iterate and find shader with correct name and type
    for ( Shader s : m_shaders )
    {
        if ( s.m_shaderName == name )
            if ( s.m_type == type )
                return s.m_shaderAddress;
    }

    return 0;
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
        if ( !exists(file) )
            continue;
        
        std::string fileName = std::filesystem::path(file).filename();
        std::string::size_type s = fileName.rfind("/");
        if ( s != std::string::npos )
        {
            fileName.erase( fileName.begin(), fileName.begin() + s );
        }

        std::string shaderName = fileName;
        s = shaderName.find(".");
        if ( s != std::string::npos )
        {
            shaderName.erase( shaderName.begin() + s, shaderName.end() );
        }

        std::string shaderType = fileName;
        s = shaderType.rfind(".");
        if ( s != std::string::npos )
        {
            shaderType.erase( shaderType.begin() + s, shaderType.end() );
        }
        s = shaderType.find(".");
        if ( s != std::string::npos )
        {
            shaderType.erase( shaderType.begin(), shaderType.begin() + s + 1 );
        }

        Shader::ShaderType type;
        if ( !shaderType.compare("vert") )
            type = Shader::shaderTypeVertex;
        else if ( !shaderType.compare("frag") )
            type = Shader::shaderTypeFragment;
        else if ( !shaderType.compare("geom") )
            type = Shader::shaderTypeGeometry;
        else if ( !shaderType.compare("comp") )
            type = Shader::shaderTypeCompute;
        else
        {
            Logger::LogLine( Logger::severityWarning, "shader {} invalid type extension {}", 
                             fileName, shaderType );
            continue;
        }

        GLuint shader;
        switch (type)
        {
            case Shader::shaderTypeVertex:
                shader = glCreateShader(GL_VERTEX_SHADER);
                break;
            case Shader::shaderTypeFragment:
                shader = glCreateShader(GL_FRAGMENT_SHADER);
                break;
            case Shader::shaderTypeGeometry:
                shader = glCreateShader(GL_GEOMETRY_SHADER);
                break;
            case Shader::shaderTypeCompute:
                shader = glCreateShader(GL_COMPUTE_SHADER);
                break;
        }

        Logger::LogLine( Logger::severityInfo, "Parsing shader {}", fileName );
        std::ifstream stream( std::filesystem::path(file).string() );

        std::string shaderText;
        while (stream) 
        {
            std::string line;
            std::getline( stream, line );
            shaderText += line + "\n";
        }

        Logger::LogLine( Logger::severityInfo, "Compiling shader {}", fileName );

        const char * p_shaderCString = shaderText.c_str();
        glShaderSource( shader, 1, &p_shaderCString, nullptr );
        glCompileShader(shader);

        if ( !VerifyShader(shader) )
        {
            glDeleteShader(shader);
            continue;
        }

        m_shaders.push_back( Shader( shader, shaderName, type ) );
    }
}

void ShaderLibrary::DeleteShaders()
{
    for ( const Shader& s : m_shaders )
    {
        glDeleteShader(s.m_shaderAddress);
    }
    m_shaders.clear();
}