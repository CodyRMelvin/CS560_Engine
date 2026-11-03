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

// Virtual class for rendering commands
class ICommand
{
    public:
        ICommand() {}
        virtual ~ICommand() {}
        virtual void Execute() = 0;
};

// Struct detailing settings for render device
struct RenderDeviceSettings
{
    GLbitfield clearOptions = 0;
    glm::vec4 clearColor = glm::vec4{0};
    bool renderSkeleton = false;
    bool renderMesh = true;
    bool pauseAnimation = true;
    bool renderSkinned = true;
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

        // Initiallize elements needed for rendering
        void Initialize();
        // Execute currently posessed commands
        void ExecuteCommands();
        // Add new command
        void Push( ICommand* cmd, size_t buffer = 0 );

        // Singleton accessor
        static RenderDevice& Get();

    private:
        // Array of buffers, buffers are arrays of commands
        std::vector< std::vector<ICommand*> > m_commandBuffers;

        // Hide constructor to force singleton
        RenderDevice() = default;
};


#endif
