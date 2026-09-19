//**************************************************************************************************
// File   : Shader.cpp
// Author : Cody Melvin
// Date   : 2026/09/18
// Brief  : Implementation of shader/progam abstraction object
//**************************************************************************************************

#include "Shader.h"

#include "Logger.h"

#include <filesystem>


Shader::Shader()
    : m_shader(0)
    , m_program(0)
    , m_name("")
    , m_type(shaderTypeInvalid)
{ }

Shader::~Shader()
{ }

void Shader::LoadShader(std::filesystem::directory_entry path)
{
        if ( !exists(path) )
            return;
        
        std::string fileName = std::filesystem::path(path).filename();
        std::string::size_type s = fileName.rfind("/");
        if ( s != std::string::npos )
        {
            fileName.erase( fileName.begin(), fileName.begin() + s );
        }

        m_name = fileName;
        s = m_name.find(".");
        if ( s != std::string::npos )
        {
            m_name.erase( m_name.begin() + s, m_name.end() );
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

        if ( !shaderType.compare("vert") )
            m_type = Shader::shaderTypeVertex;
        else if ( !shaderType.compare("frag") )
            m_type = Shader::shaderTypeFragment;
        else if ( !shaderType.compare("geom") )
            m_type = Shader::shaderTypeGeometry;
        else if ( !shaderType.compare("comp") )
            m_type = Shader::shaderTypeCompute;
        switch (m_type)
         {
            case Shader::shaderTypeVertex:
                m_shader = glCreateShader(GL_VERTEX_SHADER);
                break;
            case Shader::shaderTypeFragment:
                m_shader = glCreateShader(GL_FRAGMENT_SHADER);
                break;
            case Shader::shaderTypeGeometry:
                m_shader = glCreateShader(GL_GEOMETRY_SHADER);
                break;
            case Shader::shaderTypeCompute:
                m_shader = glCreateShader(GL_COMPUTE_SHADER);
                break;
            default:
                Logger::LogLine( Logger::severityWarning, "shader {} invalid type extension {}", 
                                 fileName, shaderType );
                return;
        }
 
        Logger::LogLine( Logger::severityInfo, "Parsing shader {}", fileName );
        std::ifstream stream( std::filesystem::path(path).string() );

        std::string shaderText;
        while (stream) 
        {
            std::string line;
            std::getline( stream, line );
            shaderText += line + "\n";
        }

        Logger::LogLine( Logger::severityInfo, "Compiling shader {}", fileName );

        const char * p_shaderCString = shaderText.c_str();
        glShaderSource( m_shader, 1, &p_shaderCString, nullptr );
        glCompileShader(m_shader);

        Logger::LogLine( Logger::severityInfo, "Creating program {}", fileName );
        m_program = glCreateProgram();
        glAttachShader( m_program, m_shader );
        glProgramParameteri( m_program, GL_PROGRAM_SEPARABLE, GL_TRUE );
        glLinkProgram(m_program);

        char buffer[512];
        GLsizei length;
        glGetProgramInfoLog( m_program, 512, &length, buffer );
        if (length) Logger::LogLine(Logger::severityError, "{}", buffer );

        GLint result;
    glValidateProgram(m_program);
    glGetProgramiv( m_program, GL_VALIDATE_STATUS, &result );
    if ( result == GL_FALSE )
    {
    GLint size = 0;
    glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &size );

    if ( size == 0 )
    {
        Logger::LogLine( Logger::severityWarning, "Program validation failed, unknown reason" );
    }
    else
    {
        
    //Preallocate a character vector of size length
    char * errorMSG = new char[size];

    //Write log to vector string
    glGetShaderInfoLog( m_program, size, &size, errorMSG );

    //Print and return
    Logger::LogLine( Logger::severityWarning, "Program validatio failed: {}", errorMSG );
    }
    }

}

void Shader::LoadShader(std::string path)
{
    LoadShader( std::filesystem::directory_entry( std::filesystem::path(path) ) );
}

void Shader::Destroy()
{
    glDeleteShader(m_shader);
    glDeleteProgram(m_program);
    Logger::LogLine( Logger::severityError, "Test" );
}

void Shader::Link() const
{
    glLinkProgram(m_program);
}

bool Shader::Verify() const
{
    //Check the compile status of the shader
    GLint compStatus = 0;
    glGetShaderiv( m_shader, GL_COMPILE_STATUS, &compStatus );

    if (compStatus == GL_TRUE)
        return true;

    //Get length of log string
    GLint size = 0;
    glGetShaderiv( m_shader, GL_INFO_LOG_LENGTH, &size );

    if ( size == 0 )
    {
        Logger::LogLine( Logger::severityWarning, "Shader compilation failed, unknown reason" );
        return false;
    }
        
    //Preallocate a character vector of size length
    char * errorMSG = new char[size];

    //Write log to vector string
    glGetShaderInfoLog( m_shader, size, &size, errorMSG );

    //Print and return
    Logger::LogLine( Logger::severityWarning, "Shader compilation failed: {}", errorMSG );

    delete[] errorMSG;
    return false;
}
