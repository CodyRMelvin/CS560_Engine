//**************************************************************************************************
// File   : Main.cpp
// Author : Cody Melvin
// Date   : 2025/06/13
// Brief  : Entry point of program
//**************************************************************************************************

#define WIRE 0
#define FLATSHADE 1

#include "Logger.h"
#include "ShaderLibrary.h"
#include "Camera.h"
#include "Math.h"
#include "Mesh.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_hints.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <filesystem>

int main ( int argc, char* argv[] )
{
    Logger::Initialize(Logger::severityInfo);
    Logger::LogLine( Logger::severityInfo, "Logger Initialized" );

    SDL_Window* p_window = nullptr;
    SDL_GLContext p_context = nullptr;
    
    int result = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    if ( result < 0 )
    {
        Logger::LogLine( Logger::severityError, "SDL_Init error: {}", SDL_GetError() );
        Logger::Shutdown();
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    
    p_window = SDL_CreateWindow( "Window", 800, 600, SDL_WINDOW_OPENGL );
    if (!p_window)
    {
        Logger::LogLine( Logger::severityError, "SDL_CreateWindow error: {}", SDL_GetError() );
        Logger::Shutdown();
        return -1;
    }

    p_context = SDL_GL_CreateContext(p_window);
    if (!p_context)
    {
        Logger::LogLine( Logger::severityError, "SDL_CreateContext error: {}", SDL_GetError() );
        Logger::Shutdown();
        return -1;
    }

    if ( !gladLoadGL() )
    {
        Logger::LogLine( Logger::severityError, "gladLoadGL failure" );
        Logger::Shutdown();
        return -1;
    }

    Logger::LogLine( Logger::severityInfo, "SDL3 intialized" );

    SDL_GL_SetSwapInterval(1);
    
    SDL_Event e;
    bool quit = false;

    std::filesystem::path dir = std::filesystem::current_path();
    std::string pathStr = dir.string();
    std::string::size_type s = pathStr.find("/bin");
    if ( s != std::string::npos )
    {
        pathStr.erase( pathStr.begin() + s, pathStr.end() );
    }
    pathStr += "/shaders";
    ShaderLibrary::CompileShaders(pathStr);

    //Create a program object, attach the shaders, and link
    GLuint program = glCreateProgram();
#if FLATSHADE
    glAttachShader( program, ShaderLibrary::GetShaderAddress("FlatShade", Shader::shaderTypeVertex) );
    glAttachShader( program, ShaderLibrary::GetShaderAddress("FlatShade", Shader::shaderTypeFragment) );
#else
    glAttachShader( program, ShaderLibrary::GetShaderAddress("demo", Shader::shaderTypeVertex) );
    glAttachShader( program, ShaderLibrary::GetShaderAddress("demo", Shader::shaderTypeFragment) );
#endif
    //Linking a program adds the shaders aas executables that can be called
    //Additionally, it sets all uniforms to 0, and allows them to be queried by name
    glLinkProgram(program);

    //After linking, allocated shader objects are no longer needed
    ShaderLibrary::DeleteShaders();

    //Check the compilation status of the program
    GLint status = 0;
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if (status == GL_FALSE)
    {
        GLint size = 0;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &size );
        std::vector<char> errorMSG;
        errorMSG.reserve(size);
        glGetProgramInfoLog( program, size, &size, errorMSG.data() );
        Logger::LogLine( Logger::severityError, "OpenGL program error: {}", errorMSG.data() );
        Logger::Shutdown();
        
        glDeleteProgram(program);
        return -1;
    }

    //Tell opengl this program is currently being used
    glUseProgram(program);

    glm::vec3 v0 = glm::vec3( -0.5, -0.5,    0 );
    glm::vec3 v1 = glm::vec3(  0.5, -0.5,    0 );
    glm::vec3 v2 = glm::vec3(    0,  0.5,    0 );
    glm::vec3 v3 = glm::vec3(    0,    0, -0.5 );

    glm::vec3 p_triangle[] = 
    {
        v0, v2, v1, v0, v3, v1, v0, v2, v3, v1, v3, v2 
    };

    std::vector<uint32_t> p_indicies = 
    {
        //Back face
        0, 1, 2,
        //Bottom face
        3, 4, 5, 
        //left face
        6, 7, 8, 
        //right face
        9, 10, 11 
    };

    glm::vec3 norm0 = glm::cross( p_triangle[1] - p_triangle[0], p_triangle[2] - p_triangle[0] );
    glm::vec3 norm1 = glm::cross( p_triangle[4] - p_triangle[5], p_triangle[3] - p_triangle[5] );
    glm::vec3 norm2 = glm::cross( p_triangle[7] - p_triangle[8], p_triangle[6] - p_triangle[8] );
    glm::vec3 norm3 = glm::cross( p_triangle[10] - p_triangle[11], p_triangle[9] - p_triangle[11] );

    glm::vec3 p_normals[] =
    {
        norm0, norm0, norm0,
        norm1, norm1, norm1, 
        norm2, norm2, norm2,
        norm3, norm3, norm3
    };

    std::vector<Vertex> prism;
    for( uint32_t i = 0; i < glm::countof(p_triangle); ++i )
    {
        Vertex v;
        v.pos = p_triangle[i];
        v.nrm = p_normals[i];
        v.tex = glm::vec2( 0, 0 );
        prism.push_back(v);
    }

    Mesh m( prism, p_indicies );
    
    glEnable(GL_DEPTH);

    Camera cam( 70.f, 4.f / 3.f, .1f, 10.f, glm::vec4( 0, 1, 0, 0 ), 
                glm::vec4( 0, 0, 0, 1 ), glm::vec4( 0, 0, 1, 0 ) );

#if FLATSHADE
    glm::mat4 normalMatrix = cam.Model();
    normalMatrix[3][0] = normalMatrix[3][1] = 0;
    normalMatrix = AffineInverse(normalMatrix);
    normalMatrix = glm::transpose(normalMatrix);

    glUniformMatrix4fv( 3, 1, GL_FALSE, glm::value_ptr( normalMatrix ) );
#endif

    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( cam.View() ) );
    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( cam.Perspective() ) );

    while (!quit)
    {
        glClearDepth(1);
        glClearColor( 0, 0, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        // glClear( GL_COLOR_BUFFER_BIT );

        static float s_rot = 0;
        s_rot += M_PI / 128;

        glm::mat4 rotMatrix = {1};
        rotMatrix = glm::translate( rotMatrix, glm::vec3( 0, 0, 2.5 ) );
        rotMatrix = glm::rotate( rotMatrix, s_rot, { 0, 1, 0} );

        // static float s_scale = 1;
        // scale += .2;
        // glm::mat4 rotMatrix = {1};
        // rotMatrix[0][0] = rotMatrix[1][1] = s_scale;
    
        glUniformMatrix4fv( 0, 1, GL_FALSE, glm::value_ptr(rotMatrix) );

        m.Draw();

        glFlush();
        
        SDL_GL_SwapWindow(p_window);

        while ( SDL_PollEvent(&e) )
        {
            switch(e.type)
            {
                case SDL_EVENT_QUIT:
                    Logger::LogLine( Logger::severityInfo, "Event quit" );
                    quit = true;
                    break;
                
                case SDL_EVENT_KEY_DOWN:
                    if (e.key.key == SDLK_W)
                        cam.Forwards(.05);
                    else if (e.key.key == SDLK_S)
                        cam.Forwards(-.05);
                    else if (e.key.key == SDLK_D)
                        cam.Strafe(.05);
                    else if (e.key.key == SDLK_A)
                        cam.Strafe(-.05);
                    
                    if (e.key.key == SDLK_UP)
                        cam.Rotate( { 1, 0, 0 }, -.02 );
                    else if (e.key.key == SDLK_DOWN)
                        cam.Rotate( { 1, 0, 0 }, .02 );
                    else if (e.key.key == SDLK_LEFT)
                        cam.Rotate( { 0, 1, 0 }, -.02 );
                    else if (e.key.key == SDLK_RIGHT)
                        cam.Rotate( { 0, 1, 0 }, .02 );

                    if ( e.key.key == SDLK_Q )
                    {
                        SDL_Event eQuit;
                        SDL_zero(eQuit);
                        eQuit.type = SDL_EVENT_QUIT;
                        SDL_PushEvent(&eQuit);
                    }

                    

                    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( cam.Model() ) );
                    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( cam.Perspective() ) );
                    break;
            }
        }
    }

    Logger::LogLine( Logger::severityInfo, "SDL_Shutdown" );
    SDL_GL_DestroyContext(p_context);
    SDL_DestroyWindow(p_window);
    
    //Delete the program, vertex buffer, and vao
    glDeleteProgram(program);

    return 0;
}


