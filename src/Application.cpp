//**************************************************************************************************
// File   : Application.cpp
// Author : Cody Melvin
// Date   : 2026/10/13
// Brief  : Implementation of class which handles application runtime 
//**************************************************************************************************

#include "Application.h"

#include "Common.h"
#include "RenderDevice.h"
#include "ShaderLibrary.h"
#include "Parser.h"
#include "Object.h"
#include "Camera.h"
#include "Pipeline.h"

#include "glad/glad.h"
#include "SDL3/SDL_events.h"

static Object obj;

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
    RenderDevice::Get().Initialize();
    
    //Create a pipeline object, attach the shaders, and link
    Pipeline pipeline;
    pipeline.Initialize(); 
    pipeline.Shader( ShaderLibrary::GetShader( "FlatShade", Shader::shaderTypeVertex ) )
            .Shader( ShaderLibrary::GetShader( "FlatShade", Shader::shaderTypeFragment ) );

    //Tell opengl this pipeline is currently being used
    pipeline.Bind();
    if(!pipeline)
    {

        GLint logLength;
        glGetProgramPipelineiv(pipeline.GLPipeline(), GL_INFO_LOG_LENGTH, &logLength);
        if ( logLength > 0 )
        {
            std::vector<char> log(logLength);
            glGetProgramPipelineInfoLog(pipeline.GLPipeline(), logLength, nullptr, log.data());    
            std::runtime_error e( std::format( "Failed to construct pipeline: {}", log.data() ) );
            throw(e);
        }

        std::runtime_error e("Failed to construct pipeline");
        throw(e);
    }

    std::filesystem::path dir( GetCWD().string() + "/Assets/glTF/roman_D_out/roman_D.gltf" );   
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
    obj.pipeline = pipeline;

    m_activeCamera.SetViewingDimensions( 90.f, 4.f / 3.f )
                  .SetPlaneDistances( .1f, 100.f )
                  .LookAt( { 0, 0, 10 }, { 0, 0, 0 }, { 0, 1, 0 } );
}

void Application::Run()
{
    SDL_Event e;
    Logger::LogLine( Logger::severityInfo, "Beginning program loop" );

    while (!m_shouldQuit)
    {
        obj.Render();
        RenderDevice::Get().ExecuteCommands();
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
                    break;
            }
        }
    }
}

void Application::Shutdown()
{
    Logger::LogLine( Logger::severityInfo, "Shutting down application" );

    //Temporary, static objects
    obj.Destroy();

    ShaderLibrary::DeleteShaders();
    Logger::Shutdown();
}

Application& Application::Get()
{
    static Application a;
    return a;
}
