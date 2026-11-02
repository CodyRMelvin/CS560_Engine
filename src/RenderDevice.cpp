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
    static RenderDevice r;
    return r;
}

void RenderDevice::Initialize()
{
    if ( !gladLoadGL() )
    {
        std::runtime_error e("gladLoadGL failure");
        throw(e);
    }

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);
#endif

    std::filesystem::path dir = GetCWD();
    std::string pathStr = dir.string();
    pathStr += "/Assets";
    ShaderLibrary::CompileShaders( pathStr + "/shaders" );

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
}

void RenderDevice::ExecuteCommands()
{
    glClearDepth(1);
    glClearColor( config.clearColor.x, config.clearColor.y, config.clearColor.z, config.clearColor.a );
    glClear(config.clearOptions);

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

    glFlush();
}

void RenderDevice::Push( ICommand* cmd, size_t buffer )
{
    if ( buffer > m_commandBuffers.size() )
        return;

    if ( buffer == m_commandBuffers.size() )
        m_commandBuffers.push_back( std::vector<ICommand*>() );

    m_commandBuffers[buffer].push_back(cmd);
}
