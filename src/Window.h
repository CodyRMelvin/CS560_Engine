//**************************************************************************************************
// File   : Window.h
// Author : Cody Melvin
// Date   : 2026/09/18
// Brief  : Interface for program window 
//**************************************************************************************************

#pragma once
#include <immintrin.h>
#ifndef WINDOW_H
#define WINDOW_H

#include "glm/glm.hpp"

// Forward declarations
struct SDL_Window;
struct SDL_GLContextState;
typedef SDL_GLContextState *SDL_GLContext;

class Window
{
    public:
        // Constructors and destructor
        Window();
        Window(glm::uvec2 res);
        Window( uint32_t width, uint32_t height );        
        ~Window() = default;

        // Initialize SDL and create window
        void Initialize();
        // Destroy all window resources
        void Destroy();
        // Update the window
        void SwapWindow() const;
        // Resizes the window
        void Resize( glm::uvec2 res );

        // Getters
        float AspectRatio() { return float(m_res.x) / float(m_res.y); }
        SDL_Window* SDLWindow() const { return m_p_window; }
        SDL_GLContext Context() const { return m_p_context; }

    private:
        glm::uvec2 m_res;
        SDL_Window* m_p_window;
        SDL_GLContext m_p_context;
};

#endif
