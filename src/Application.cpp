//**************************************************************************************************
// File   : Application.cpp
// Author : Cody Melvin
// Date   : 2026/10/13
// Brief  : Implementation of class which handles application runtime 
//**************************************************************************************************

#include "Application.h"

#include "Common.h"
#include "ShaderLibrary.h"
#include "Parser.h"
#include "Object.h"
#include "Mesh.h"
#include "Camera.h"
#include <immintrin.h>
#include <stdexcept>
#ifdef _DEBUG
#include "Debug.h"
#endif

#include "glad/glad.h"
#include "SDL3/SDL.h"

static Object obj;
static GLuint pipeline;

Application::Application()
    : m_window()
    , m_activeCamera() 
    , m_shouldQuit(false)
{ }

Application::~Application()
{ };

void Application::Initialize()
{
    Logger::Initialize(Logger::severityInfo);
    Logger::LogLine( Logger::severityInfo, "Logger Initialized" );

    m_window.Initialize();
    
    if ( !gladLoadGL() )
    {
        std::runtime_error e("gladLoadGL failure");
        throw(e);
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);

    std::filesystem::path dir = GetCWD();
    std::string pathStr = dir.string();
    std::string::size_type size = pathStr.find("/bin");
    if ( size != std::string::npos )
    {
        pathStr.erase( pathStr.begin() + size, pathStr.end() );
    }
    pathStr += "/Assets";
    glEnableVertexAttribArray(0);
    ShaderLibrary::CompileShaders( pathStr + "/shaders" );

    //Create a pipeline object, attach the shaders, and link
    glGenProgramPipelines( 1, &pipeline );

    Shader vs = ShaderLibrary::GetShader("FlatShade", Shader::shaderTypeVertex );
    vs.Link();
    glUseProgramStages( pipeline, GL_VERTEX_SHADER_BIT, vs.Program() );
    Shader fs = ShaderLibrary::GetShader("FlatShade", Shader::shaderTypeFragment);
    fs.Link();
    glUseProgramStages( pipeline, GL_FRAGMENT_SHADER_BIT, fs.Program() );

    //Tell opengl this pipeline is currently being used
    glBindProgramPipeline(pipeline);

    dir = std::filesystem::path( pathStr + "/glTF/roman_D_out/roman_D.gltf" );   
    Parser parser( dir, fastgltf::Options::LoadExternalBuffers );
    if (!parser)
    {
        std::runtime_error e("Failed to construct parser");
        throw(e);
    }

    std::vector<Mesh> meshes;
    if ( !parser.ReadMeshes(meshes) )
    {
        std::runtime_error e("Failed to parse mesh");
        throw(e);
    }

    obj = Object( meshes[0], Transform( 1, 1, 1 ) );
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    m_activeCamera.SetViewingDimensions( 90.f, 4.f / 3.f )
                  .SetPlaneDistances( .1f, 100.f )
                  .LookAt( { 0, 0, 10 }, { 0, 0, 0 }, { 0, 1, 0 } );
}

void Application::Run()
{
    Shader vs = ShaderLibrary::GetShader("FlatShade", Shader::shaderTypeVertex );
    glUseProgram( vs.Program() );
    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( m_activeCamera.View() ) );
    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( m_activeCamera.Perspective() ) );
    glm::mat4 normalMatrix = m_activeCamera.Model();
    normalMatrix[3][0] = normalMatrix[3][1] = 0;
    normalMatrix = AffineInverse(normalMatrix);
    normalMatrix = glm::transpose(normalMatrix);
    glUniformMatrix4fv( 3, 1, GL_FALSE, glm::value_ptr( normalMatrix ) );
    glUseProgram(0);

    SDL_Event e;
    Logger::LogLine( Logger::severityInfo, "Beginning program loop" );
    while (!m_shouldQuit)
    {
        glClearDepth(1);
        glClearColor( 0, 0, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        obj.Render( vs.Program() );

        glFlush();

        m_window.SwapWindow();

        while ( SDL_PollEvent(&e) )
        {
            switch(e.type)
            {
                case SDL_EVENT_QUIT:
                    Logger::LogLine( Logger::severityInfo, "Event quit" );
                    m_shouldQuit = true;
                    break;
                
                case SDL_EVENT_KEY_DOWN:
                    if (e.key.key == SDLK_W)
                        m_activeCamera.Forwards(.5);
                    if (e.key.key == SDLK_S)
                        m_activeCamera.Forwards(-.5);
                    if (e.key.key == SDLK_D)
                        m_activeCamera.Strafe(.5);
                    if (e.key.key == SDLK_A)
                        m_activeCamera.Strafe(-.5);
                    
                    if (e.key.key == SDLK_UP)
                        m_activeCamera.Rotate( { 1, 0, 0 }, .05 );
                    if (e.key.key == SDLK_DOWN)
                        m_activeCamera.Rotate( { 1, 0, 0 }, -.05 );
                    if (e.key.key == SDLK_LEFT)
                        m_activeCamera.Rotate( { 0, 1, 0 }, .05 );
                    if (e.key.key == SDLK_RIGHT)
                        m_activeCamera.Rotate( { 0, 1, 0 }, -.05 );

                    if ( e.key.key == SDLK_Q )
                    {
                        SDL_Event eQuit;
                        SDL_zero(eQuit);
                        eQuit.type = SDL_EVENT_QUIT;
                        SDL_PushEvent(&eQuit);
                    }

                    glUseProgram( vs.Program() );
                    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( m_activeCamera.View() ) );
                    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( m_activeCamera.Perspective() ) );
                    glm::mat4 normalMatrix = m_activeCamera.Model();
                    normalMatrix[3][0] = normalMatrix[3][1] = 0;
                    normalMatrix = AffineInverse(normalMatrix);
                    normalMatrix = glm::transpose(normalMatrix);
                    glUniformMatrix4fv( 3, 1, GL_FALSE, glm::value_ptr( normalMatrix ) );
                    glUseProgram(0);
                    break;
            }
        }
    }

}

void Application::Shutdown()
{
    Logger::LogLine( Logger::severityInfo, "Shutting down application" );
    //Delete the program, vertex buffer, and vao
    glDeleteProgramPipelines( 1, &pipeline );
    ShaderLibrary::DeleteShaders();
    obj.Destroy();
    Logger::Shutdown();
}

