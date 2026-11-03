//**************************************************************************************************
// File   : RenderDevice.cpp
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Implementation of system which handles rendering of objects  
//**************************************************************************************************

#include "RenderDevice.h"
#include "ShaderLibrary.h"
#include "Animation.h"
#ifdef _DEBUG
#include "Debug.h"
#endif

#include <algorithm>

RenderDevice& RenderDevice::Get()
{
    // Get the singleton render device
    static RenderDevice r;
    return r;
}

void RenderDevice::Initialize()
{
    // Load glad and throw error if loading fails
    if ( !gladLoadGL() )
    {
        std::runtime_error e("gladLoadGL failure");
        throw(e);
    }

    // If in debug mode, enable opengl debug messages
#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);
#endif

    // Compile shaders
    std::filesystem::path dir = GetCWD();
    std::string pathStr = dir.string();
    pathStr += "/Assets";
    ShaderLibrary::CompileShaders( pathStr + "/shaders" );

    // Enable gl settings
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
}

void RenderDevice::ExecuteCommands()
{
    // Clear frame
    glClearDepth(1);
    glClearColor( config.clearColor.x, config.clearColor.y, config.clearColor.z, config.clearColor.a );
    glClear(config.clearOptions);

    // Execute each command in each buffer
    std::for_each( m_commandBuffers.begin(), m_commandBuffers.end(), 
    []( std::vector<ICommand*>& buffer )
    { 
        std::for_each( buffer.begin(), buffer.end(),
        []( ICommand*& cmd )
        {
            if (!cmd)
                return;

            cmd->Execute(); 
            delete cmd;
            cmd = nullptr;
        });

        buffer.clear();
    });

    // Flush the opengl buffers
    glFlush();
}

void RenderDevice::Push( ICommand* cmd, size_t buffer )
{
    // If buffer index is too large, return
    if ( buffer > m_commandBuffers.size() )
        return;

    // Append new buffer if one off
    if ( buffer == m_commandBuffers.size() )
        m_commandBuffers.push_back( std::vector<ICommand*>() );

    // Add command to buffer
    m_commandBuffers[buffer].push_back(cmd);
}
