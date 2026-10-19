//**************************************************************************************************
// File   : Window.h
// Author : Cody Melvin
// Date   : 2026/09/18
// Brief  : Interface for program window 
//**************************************************************************************************

#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include "glm/glm.hpp"

struct SDL_Window;
struct SDL_GLContextState;
typedef SDL_GLContextState *SDL_GLContext;

class Window
{
    public:
        Window();
        Window(glm::uvec2 res);
        Window( uint32_t width, uint32_t height );        
        ~Window() = default;

        void Initialize();
        void Destroy();
        void SwapWindow() const;

        float AspectRatio() { return float(m_res.x) / float(m_res.y); }

    private:
        glm::uvec2 m_res;
        SDL_Window* m_p_window;
        SDL_GLContext m_p_context;
};

#endif
