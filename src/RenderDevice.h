//**************************************************************************************************
// File   : RenderDevice.h
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Interface for system which handles rendering
//**************************************************************************************************

#pragma once
#ifndef RendererDeviceRendererDevice_H
#define RendererDeviceRendererDevice_H

#include "Common.h"

#include "glad/glad.h"

class ICommand
{
    public:
        ICommand() {}
        virtual ~ICommand() {}
        virtual void Execute() = 0;
};

struct RenderDeviceSettings
{
    GLbitfield clearOptions;
    glm::vec4 clearColor;
};

class RenderDevice
{
    public:
        RenderDeviceSettings config;

        // Force default destructor
        ~RenderDevice() = default;

        // Force deletion of copying, moving, and assignment semantics
        RenderDevice( const RenderDevice& ) = delete;
        RenderDevice( const RenderDevice&& ) = delete;
        RenderDevice& operator=( const RenderDevice& ) = delete;

        void Initialize();
        void ExecuteCommands();
        void Push( ICommand* cmd );

        // Singleton method
        static RenderDevice& Get();

    private:
        std::vector<ICommand*> m_commandBuffer;

        // Hide constructor to force singleton
        RenderDevice() = default;
};


#endif
