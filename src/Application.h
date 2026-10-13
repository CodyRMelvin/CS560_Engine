//**************************************************************************************************
// File   : Application.h
// Author : Cody Melvin
// Date   : 2026/10/13
// Brief  : Interface for class which handles application runtime
//**************************************************************************************************

#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "Camera.h"

#include <filesystem>

class Application
{
    public:
        Application();
        ~Application();

        void Initialize();
        void Run();
        void Shutdown();

        static std::filesystem::path GetCWD()
        {
            static std::filesystem::path dir  = std::filesystem::current_path();
            return dir; 
        }

    private:
        Window m_window;
        Camera m_activeCamera;
        bool m_shouldQuit;
};

#endif
