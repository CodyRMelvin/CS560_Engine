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
#include "Animation.h"

#include "fastgltf/core.hpp"
#include "glad/glad.h"
#include "SDL3/SDL_events.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "SDL3/SDL_timer.h"

#include <algorithm>
#include <stdexcept>
#include <typeinfo>

// Static variables used by gui
static int activeAnimation = 0;
static int loadedFile = 0;
static bool reload = false;

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
    // Create full path and initialize parser for the desired gltf file
    std::filesystem::path dir( GetCWD().string() + assetDir );   
    Parser parser( dir, fastgltf::Options::LoadExternalBuffers | fastgltf::Options::DecomposeNodeMatrices );
    if (!parser)
    {
        std::runtime_error e("Failed to construct parser");
        throw(e);
    }

    // Read contents of file into program
    bool res = parser.ReadScene();
    if (!res)
    {
        std::runtime_error e("Failed to read scene");
        throw(e);
    }

    // Initialize each node 
    std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(), 
    []( auto& pair )
    {
        pair.second->Initialize();
    });

    // Find first camera if one is present in scene
    auto camIt = std::find_if( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
    []( auto& pair )
    {
        return typeid(*pair.second) == typeid(Camera); 
    });

    // Set active camera if one is found, otherwise construct default camera
    if ( camIt != Node::NodeLibrary.end() )
        m_activeCamera = *static_cast<Camera*>(camIt->second);
    else
    {
        m_activeCamera.SetViewingDimensions( 90.f, m_window.AspectRatio() )
                      .SetPlaneDistances( .1f, 1000.f )
                      .LookAt( { 0, .5, 1.5 }, { 0, 1, 0 }, { 0, 1, 0 } );
    }

    // Initialize each animation
    std::for_each( Animation::AnimationLibrary.begin(), Animation::AnimationLibrary.end(), 
    []( Animation& anim )
    {
        anim.Initialize();
    });
    // Set default active animation
    activeAnimation = 0;
}

void Application::Initialize()
{
    // Initialize logging system, info in debug mode, warnings in release
#ifdef _DEBUG
    Logger::Initialize(Logger::severityInfo);
#else
    Logger::Initialize(Logger::severityWarning);
#endif
    Logger::LogLine( Logger::severityInfo, "Logger Initialized" );

    // Hard coded list of scenes to switch between
    m_files.push_back("roman/roman_D.gltf");
    m_files.push_back("viking_C_out/viking_C.gltf");

    // Initialize window
    m_window.Initialize();

    // Initialize DearImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    style.FontSizeBase = 16.f;
    style.ScaleAllSizes( SDL_GetDisplayContentScale( SDL_GetPrimaryDisplay() ) );

    ImGui_ImplSDL3_InitForOpenGL( m_window.SDLWindow(), m_window.Context() ); 
    ImGui_ImplOpenGL3_Init();

    // Initialize the render device and set default config
    RenderDevice::Get().Initialize();
    RenderDevice::Get().config = 
    { 
        .clearOptions = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 
        .clearColor = { .3, .2, 1, 1 },
        .renderSkeleton = true,
        .renderMesh = true,
    };

    // Set up the first scene
    SetUpScene( std::format( "/Assets/glTF/{}", m_files[loadedFile] ) );
    // Initialize previous value for dt calculations
    m_previous = std::chrono::steady_clock::now();
}

void Application::Run()
{
    SDL_Event e;
    Logger::LogLine( Logger::severityInfo, "Beginning program loop" );

    // Program loop
    while (!m_shouldQuit)
    {
        // Reload scene if needed
        if (reload)
        {
            DeleteAllNodes();
            SetUpScene( std::format( "/Assets/glTF/{}", m_files[loadedFile] ) );
            reload = false;
        }

        // Calculate delta time
        m_previous = m_now;
        m_now = std::chrono::steady_clock::now();
        m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>( m_now - m_previous ).count() / 1000.f;

        // Update active animation if not paused
        if ( !RenderDevice::Get().config.pauseAnimation )
            Animation::AnimationLibrary[activeAnimation].Animate();

        // Render each node
        std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
        []( auto& pair )
        {
            if (pair.second)
                pair.second->Render();
        });

        // Render GUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        ImGui::Begin("Config");
        // Shows framerate in window
        ImGui::Text( std::format( "FPS: {:.3}", ImGui::GetIO().Framerate ).c_str());
        // Box to toggle rending the mesh
        ImGui::Checkbox( "Render Mesh", &RenderDevice::Get().config.renderMesh );
        // Box to toggle rending the skeleton
        ImGui::Checkbox( "Render Skeleton", &RenderDevice::Get().config.renderSkeleton );
        // Box to toggle pausing animations
        ImGui::Checkbox( "Pause Animation", &RenderDevice::Get().config.pauseAnimation );
        // Box to toggle skinning the mesh
        ImGui::Checkbox( "Render Skinned", &RenderDevice::Get().config.renderSkinned );
        // Slider to scale dt, slow down or accelerate program
        ImGui::DragFloat( "Delta Time Scalar", &m_dtScalar, .01, 0, 10 );

        // Get the name of each animation
        std::vector<const char*> animations;
        for ( const Animation& anim : Animation::AnimationLibrary )
            animations.push_back( anim.Name().data() );

        // Create list box of each animation so that user may switch between them
        if ( ImGui::BeginListBox("Active Animation") )
        {
            for ( int i = 0; i < animations.size(); ++i )
            {
                const bool isSelected = activeAnimation == i;
                if ( ImGui::Selectable( animations[i], isSelected ) )
                    activeAnimation = i;
            }

            ImGui::EndListBox();
        }

        // Create list box of each available scene so that used may switch between them
        if ( ImGui::BeginListBox("Loaded Model") )
        {
            for ( int i = 0; i < m_files.size(); ++i )
            {
                const bool isSelected = loadedFile  == i;
                if ( ImGui::Selectable( m_files[i], isSelected ) )
                {
                    loadedFile  = i;
                    reload = true;
                }
            }

            ImGui::EndListBox();
        }

        ImGui::End();
        ImGui::Render();

        // Run each render command in render device
        RenderDevice::Get().ExecuteCommands();
        // Render the GUI with opengl
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
        // Swap window
        m_window.SwapWindow();

        // Manage SDL events
        while ( SDL_PollEvent(&e) )
        {
            // Pass inuts to gui
            ImGui_ImplSDL3_ProcessEvent(&e);

            switch(e.type)
            {
                // exit program
                case SDL_EVENT_QUIT:
                    Logger::LogLine( Logger::severityInfo, "Event quit" );
                    m_shouldQuit = true;
                    break;

                // Resize window
                case SDL_EVENT_WINDOW_RESIZED:
                    m_window.Resize( glm::uvec2{ e.window.data1, e.window.data2 } ); 
                    break;

                case SDL_EVENT_KEY_DOWN:
                    // Camera controls for translation
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

                    // Camera controls for rotation
                    if (e.key.key == SDLK_UP)
                        m_activeCamera.Rotate( { 1, 0, 0 }, .05 );
                    if (e.key.key == SDLK_DOWN)
                        m_activeCamera.Rotate( { 1, 0, 0 }, -.05 );
                    if (e.key.key == SDLK_LEFT)
                        m_activeCamera.Rotate( { 0, 1, 0 }, .05 );
                    if (e.key.key == SDLK_RIGHT)
                        m_activeCamera.Rotate( { 0, 1, 0 }, -.05 );

                    // Press space to toggle animation
                    if ( e.key.key == SDLK_SPACE )
                        RenderDevice::Get().config.pauseAnimation ^= true;

                    // Exit program with exc button
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

    // Shut down GUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();    

    // Destroy all stored information for application
    m_window.Destroy();
    DeleteAllNodes();
    ShaderLibrary::DeleteShaders();

    // Shutdown logger
    Logger::Shutdown();
}

Application& Application::Get()
{
    // Singleton access
    static Application a;
    return a;
}
