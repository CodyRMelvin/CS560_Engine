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
#include "Window.h"
#include "Transform.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <filesystem>

int main ( int argc, char* argv[] )
{
    Logger::Initialize(Logger::severityInfo);
    Logger::LogLine( Logger::severityInfo, "Logger Initialized" );

    Window window( 800, 600 ); 
    try { window.Initialize(); }
    catch (std::runtime_error e)
    {
        Logger::LogLine( Logger::severityError, e.what() );
        return -1;
    }
    
    if ( !gladLoadGL() )
    {
        Logger::LogLine( Logger::severityError, "gladLoadGL failure" );
        Logger::Shutdown();
        return -1;
    }

    SDL_Event e;
    bool quit = false;

    std::filesystem::path dir = std::filesystem::current_path();
    std::string pathStr = dir.string();
    std::string::size_type size = pathStr.find("/bin");
    if ( size != std::string::npos )
    {
        pathStr.erase( pathStr.begin() + size, pathStr.end() );
    }
    pathStr += "/shaders";
    ShaderLibrary::CompileShaders(pathStr);

    //Create a pipeline object, attach the shaders, and link
    GLuint pipeline;
    glGenProgramPipelines( 1, &pipeline );

#if FLATSHADE
    Shader vs = ShaderLibrary::GetShader("FlatShade", Shader::shaderTypeVertex );
    vs.Link();
    glUseProgramStages( pipeline, GL_VERTEX_SHADER_BIT, vs.Program() );
    Shader fs = ShaderLibrary::GetShader("FlatShade", Shader::shaderTypeFragment);
    fs.Link();
    glUseProgramStages( pipeline, GL_FRAGMENT_SHADER_BIT, fs.Program() );
#else
    Shader s = ShaderLibrary::GetShader("demo", Shader::shaderTypeVertex );
    s.Link();
    glUseProgramStages( pipeline, GL_VERTEX_SHADER_BIT, s.Program() );
    s = ShaderLibrary::GetShader("demo", Shader::shaderTypeFragment);
    s.Link();
    glUseProgramStages( pipeline, GL_FRAGMENT_SHADER_BIT, s.Program() );
#endif

    //Tell opengl this pipeline is currently being used
    glBindProgramPipeline(pipeline);

    glm::vec3 v0 = glm::vec3( -0.5, -0.5,    0 );
    glm::vec3 v1 = glm::vec3(  0.5, -0.5,    0 );
    glm::vec3 v2 = glm::vec3(    0,  0.5,    0 );
    glm::vec3 v3 = glm::vec3(    0,    0, -0.5 );

    glm::vec3 p_triangle[] = 
    {
        v1, v2, v0, v0, v3, v1, v0, v2, v3, v1, v3, v2 
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
    Transform t( 1, 1, 1, 0, 0, 2.5 );
    
    glEnable(GL_DEPTH);
    glEnable(GL_CULL_FACE);

    Camera cam( 70.f, 4.f / 3.f, .1f, 10.f, glm::vec4( 0, 1, 0, 0 ), 
                glm::vec4( 0, 0, 0, 1 ), glm::vec4( 0, 0, 1, 0 ) );

    glUseProgram( vs.Program() );
#if FLATSHADE
    glm::mat4 normalMatrix = cam.Model();
    normalMatrix[3][0] = normalMatrix[3][1] = 0;
    normalMatrix = AffineInverse(normalMatrix);
    normalMatrix = glm::transpose(normalMatrix);

    glUniformMatrix4fv( 3, 1, GL_FALSE, glm::value_ptr( normalMatrix ) );
#endif

    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( cam.View() ) );
    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( cam.Perspective() ) );
    glUseProgram(0);

    while (!quit)
    {
        glClearDepth(1);
        glClearColor( 0, 0, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        static float s_rot = 0;
        s_rot += M_PI / 128;
        t.Rotation( glm::vec3( 0, 1, 0 ), s_rot );

        glUseProgram( vs.Program() );
        glUniformMatrix4fv( 0, 1, GL_FALSE, glm::value_ptr( t.Matrix() ) );
        glUseProgram(0);

        m.Draw();

        glFlush();

        window.SwapWindow();

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

                    

                    glUseProgram( vs.Program() );
                    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( cam.Model() ) );
                    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( cam.Perspective() ) );
                    glUseProgram(0);
                    break;
            }
        }
    }
    
    //Delete the program, vertex buffer, and vao
    glDeleteProgramPipelines( 1, &pipeline );
    ShaderLibrary::DeleteShaders();
    Logger::Shutdown();

    return 0;
}


