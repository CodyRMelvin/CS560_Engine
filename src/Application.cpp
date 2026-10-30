//**************************************************************************************************
// File   : Application.cpp
// Author : Cody Melvin
// Date   : 2026/10/13
// Brief  : Implementation of class which handles application runtime 
//**************************************************************************************************

#include "Application.h"

#include "Common.h"
#include "RenderDevice.h"
#include "SDL3/SDL_video.h"
#include "ShaderLibrary.h"
#include "Parser.h"
#include "Camera.h"
#include "Node.h"

#include "fastgltf/core.hpp"
#include "glad/glad.h"
#include "SDL3/SDL_events.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <stdexcept>
#include <typeinfo>

bool animate = false;
int activeAnimation = 0;

Application::Application()
    : m_window()
    , m_activeCamera() 
    , m_shouldQuit(false)
    , m_deltaTime(0)
    , m_dtScalar(1)
{ }

Application::~Application()
{ };

void Application::SetUpScene( std::string assetDir )
{
    std::filesystem::path dir( GetCWD().string() + assetDir );   
    Parser parser( dir, fastgltf::Options::LoadExternalBuffers );
    if (!parser)
    {
        std::runtime_error e("Failed to construct parser");
        throw(e);
    }

    bool res = parser.ReadScene();
    if (!res)
    {
        std::runtime_error e("Failed to read scene");
        throw(e);
    }

    std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(), 
    []( auto& pair )
    {
        pair.second->Initialize();
    });

    auto camIt = std::find_if( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
    []( auto& pair )
    {
        return typeid(*pair.second) == typeid(Camera); 
    });

    if ( camIt != Node::NodeLibrary.end() )
        m_activeCamera = *static_cast<Camera*>(camIt->second);
    else
    {
        m_activeCamera.SetViewingDimensions( 90.f, m_window.AspectRatio() )
                      .SetPlaneDistances( .1f, 1000.f )
                      .LookAt( { 0, .5, 1.5 }, { 0, 1, 0 }, { 0, 1, 0 } );
    }

    std::for_each( Animation::AnimationLibrary.begin(), Animation::AnimationLibrary.end(), 
    []( Animation& anim )
    {
        anim.Initialize();
    });
    activeAnimation = 0;
}

void Application::Initialize()
{
    Logger::Initialize(Logger::severityInfo);
    Logger::LogLine( Logger::severityInfo, "Logger Initialized" );

    m_window.Initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    style.FontSizeBase = 16.f;
    style.ScaleAllSizes( SDL_GetDisplayContentScale( SDL_GetPrimaryDisplay() ) );

    ImGui_ImplSDL3_InitForOpenGL( m_window.SDLWindow(), m_window.Context() ); 
    ImGui_ImplOpenGL3_Init();

    RenderDevice::Get().Initialize();
    RenderDevice::Get().config = 
    { 
        .clearOptions = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 
        .clearColor = { .3, .2, 1, 1 },
        .renderSkeleton = true,
        .renderMesh = true,
    };

    SetUpScene("/Assets/glTF/roman_D_2/roman_D.gltf");
    //SetUpScene("/Assets/glTF/SimpleSkin/SimpleSkin.gltf");
    //SetUpScene("/Assets/glTF/RiggedFigure.glb");
    m_previous = std::chrono::steady_clock::now();
}

void Application::Run()
{
    SDL_Event e;
    Logger::LogLine( Logger::severityInfo, "Beginning program loop" );

    while (!m_shouldQuit)
    {
        m_previous = m_now;
        m_now = std::chrono::steady_clock::now();
        m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>( m_now - m_previous ).count() / 1000.f;

        if (animate)
        {
            Animation::AnimationLibrary[activeAnimation].Animate();
            /*std::for_each( Animation::AnimationLibrary.begin(), Animation::AnimationLibrary.end(), 
            []( Animation& anim )
            {
                anim.Animate();
            });*/
        }

        std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
        []( auto& pair )
        {
            if (pair.second)
                pair.second->Render();
        });

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Begin("Config");
        ImGui::Checkbox( "Render Mesh", &RenderDevice::Get().config.renderMesh );
        ImGui::Checkbox( "Render Skeleton", &RenderDevice::Get().config.renderSkeleton );
        ImGui::Checkbox( "Play Animation", &animate );
        ImGui::DragFloat( "Delta Time Scalar", &m_dtScalar, .01, 0, 10 );

        std::vector<const char*> animations;
        for ( const Animation& anim : Animation::AnimationLibrary )
            animations.push_back( anim.Name().data() );

        ImGui::Text("Active Animation");
        ImGui::ListBox( " ", &activeAnimation, animations.data(), animations.size() );

        ImGui::End();

        ImGui::Render();
        RenderDevice::Get().ExecuteCommands();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
        m_window.SwapWindow();

        while ( SDL_PollEvent(&e) )
        {
            ImGui_ImplSDL3_ProcessEvent(&e);

            switch(e.type)
            {
                case SDL_EVENT_QUIT:
                    Logger::LogLine( Logger::severityInfo, "Event quit" );
                    m_shouldQuit = true;
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                    m_window.Resize( glm::uvec2{ e.window.data1, e.window.data2 } ); 
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
                    if ( e.key.key == SDLK_Q )
                        m_activeCamera.Rise(-.5);
                    if ( e.key.key == SDLK_E )
                        m_activeCamera.Rise(.5);
                    
                    if (e.key.key == SDLK_UP)
                        m_activeCamera.Rotate( { 1, 0, 0 }, .05 );
                    if (e.key.key == SDLK_DOWN)
                        m_activeCamera.Rotate( { 1, 0, 0 }, -.05 );
                    if (e.key.key == SDLK_LEFT)
                        m_activeCamera.Rotate( { 0, 1, 0 }, .05 );
                    if (e.key.key == SDLK_RIGHT)
                        m_activeCamera.Rotate( { 0, 1, 0 }, -.05 );

                    // Press tab to toggle rendering the mesh
                    if ( e.key.key == SDLK_TAB )
                        RenderDevice::Get().config.renderMesh ^= true;
                    // Press left ctrl to toggle rendering the skeleton
                    if ( e.key.key == SDLK_LCTRL )
                        RenderDevice::Get().config.renderSkeleton ^= true;
                    // Press space to toggle animation
                    if ( e.key.key == SDLK_SPACE )
                        animate = !animate;

                    if ( e.key.key == SDLK_1 )
                    {
                        DeleteAllNodes();
                        SetUpScene("/Assets/glTF/viking_C_out/viking_C.gltf");                    
                    }
                    if ( e.key.key == SDLK_2 )
                    {
                        DeleteAllNodes();
                        SetUpScene("/Assets/glTF/roman_D_2/roman_D.gltf");   
                    }
                    if ( e.key.key == SDLK_3 )
                    {
                        DeleteAllNodes();
                        SetUpScene("/Assets/glTF/RiggedFigure.glb");
                    }
                    if ( e.key.key == SDLK_4 )
                    {
                        DeleteAllNodes();
                        SetUpScene("/Assets/glTF/SimpleSkin/SimpleSkin.gltf");
                    }
                    if ( e.key.key == SDLK_5 )
                    {
                        DeleteAllNodes();
                        SetUpScene("/Assets/glTF/roman_D_out/roman_D.gltf");   
                    }

                    if ( e.key.key == SDLK_ESCAPE )
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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();    

    m_window.Destroy();
    DeleteAllNodes();
    ShaderLibrary::DeleteShaders();

    Logger::Shutdown();
}

Application& Application::Get()
{
    static Application a;
    return a;
}
