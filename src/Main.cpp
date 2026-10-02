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
#include "Window.h"
#include "Object.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include <vector>
#include <filesystem>

void glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    Logger::LogLine( Logger::severityError, "Source: {}, type: {}, id: {}, severity: {}, length: {}, message: {}, userParam: {}",
                     source, type, id, severity, length, message, userParam );
}

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

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);

    SDL_Event e;
    bool quit = false;

    std::filesystem::path dir = std::filesystem::current_path();
    std::string pathStr = dir.string();
    std::string::size_type size = pathStr.find("/bin");
    if ( size != std::string::npos )
    {
        pathStr.erase( pathStr.begin() + size, pathStr.end() );
    }
    pathStr += "/Assets";
    ShaderLibrary::CompileShaders( pathStr + "/shaders" );

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

    fastgltf::Parser parser;
    dir = std::filesystem::path( pathStr + "/glTF/egyptian_B.gltf" );

    auto gltfStream = fastgltf::GltfDataBuffer::FromPath(dir);
    if ( gltfStream.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityError, "Failed to read gltf. Error number {}", static_cast<uint64_t>( gltfStream.error() ) );
        return -1;
    }

    auto asset = parser.loadGltf( gltfStream.get(), dir.parent_path(), fastgltf::Options::None );
    if ( asset.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityError, "Failed to parse gltf. Error number {}", static_cast<uint64_t>( gltfStream.error() ) );
        return -1;
    }

    std::vector<Object> objects;

    fastgltf::iterateSceneNodes( asset.get(), 0, fastgltf::math::fmat4x4(), 
    [&]( fastgltf::Node& node, const fastgltf::math::fmat4x4& matrix )
    {
        if ( node.meshIndex.has_value() )
        {
            fastgltf::Mesh gltfMesh = asset->meshes[*node.meshIndex];
            fastgltf::Primitive positions = gltfMesh.primitives.front();
        }
    });
    
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
        prism.transform.Rotation( glm::vec3( 0, 1, 0 ), s_rot );

        glUseProgram( vs.Program() );
        glUniformMatrix4fv( 0, 1, GL_FALSE, glm::value_ptr( prism.transform.Matrix() ) );
        glUseProgram(0);

        prism.mesh.Draw();

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
    prism.Destroy();
    Logger::Shutdown();

    return 0;
}


