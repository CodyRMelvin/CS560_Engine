//**************************************************************************************************
// File   : Window.h
// Author : Cody Melvin
// Date   : 2026/09/18
// Brief  : Interface for program window 
//**************************************************************************************************

#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>

#include "glm/glm.hpp"

class Window
{
    public:
        Window();
        Window(glm::uvec2 res);
        Window( uint32_t width, uint32_t height );        
        ~Window();

        void Initialize();
        void SwapWindow() const;


    private:
        glm::uvec2 m_res;
        SDL_Window* m_p_window;
        SDL_GLContext m_p_context;
};

#endif
