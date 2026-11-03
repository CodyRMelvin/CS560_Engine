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

class Application
{
    public:
        // Destructor
        ~Application();

        // Standard methods for using program
        void Initialize();
        void Run();
        void Shutdown();

        // Singleton getter
        static Application& Get();

        // Getters for other members variables
        Window& GetWindow() { return m_window; }
        Camera& GetActiveCamera() { return m_activeCamera; }
        float GetDeltaTime() const { return m_dtScalar * m_deltaTime; }

    private:
        Window m_window;
        Camera m_activeCamera;
        bool m_shouldQuit;
        std::chrono::time_point<std::chrono::steady_clock> m_now;
        std::chrono::time_point<std::chrono::steady_clock> m_previous;
        float m_deltaTime;
        float m_dtScalar;
        std::vector<const char *> m_files;

        // Private constructor for singleton
        Application();
        // Helper function to read in gltf file
        void SetUpScene( std::string assetDir );
};

#endif
