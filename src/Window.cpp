//**************************************************************************************************
// File   : Window.cpp
// Author : Cody Melvin
// Date   : 2026/09/18
// Brief  : Implementation for program window
//**************************************************************************************************

#include "Window.h"

#include "Logger.h"

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_hints.h>

Window::Window()
    : m_res( 800, 600 )
    , m_p_window(nullptr)
    , m_p_context(nullptr)
{ }

Window::Window(glm::uvec2 res)
    : m_res(res)
    , m_p_window(nullptr)
    , m_p_context(nullptr)
{ }

Window::Window( uint32_t width, uint32_t height )
    : m_res( width, height )
    , m_p_window(nullptr)
    , m_p_context(nullptr)
{ }

void Window::Initialize()
{
    bool result = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
    if (!result)
    {
        std::runtime_error e( std::format( "SDL_Init error {}", SDL_GetError() ) );
        throw e;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    
    m_p_window = SDL_CreateWindow( "Window", m_res.x, m_res.y, SDL_WINDOW_OPENGL );
    if (!m_p_window)
    {
        std::runtime_error e( std::format( "SDL_CreateWindow error {}", SDL_GetError() ) );
        throw e;
    }

    m_p_context = SDL_GL_CreateContext(m_p_window);
    if (!m_p_context)
    {
        std::runtime_error e( std::format( "SDL_CreateContext error {}", SDL_GetError() ) );
        throw e;
    }

    SDL_GL_SetSwapInterval(1);
    Logger::LogLine( Logger::severityInfo, "SDL3 intialized" );
}

void Window::Destroy()
{
    Logger::LogLine( Logger::severityInfo, "SDL_Shutdown" );
    SDL_GL_DestroyContext(m_p_context);
    SDL_DestroyWindow(m_p_window);
}

void Window::SwapWindow() const
{
    SDL_GL_SwapWindow(m_p_window);
}
