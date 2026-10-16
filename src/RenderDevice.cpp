//**************************************************************************************************
// File   : RenderDevice.cpp
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Implementation of system which handles rendering of objects  
//**************************************************************************************************

#include "RenderDevice.h"
#include "ShaderLibrary.h"
#ifdef _DEBUG
#include "Debug.h"
#endif

#include "glad/glad.h"

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
    std::string::size_type size = pathStr.find("/bin");
    if ( size != std::string::npos )
    {
        pathStr.erase( pathStr.begin() + size, pathStr.end() );
    }
    pathStr += "/Assets";
    glEnableVertexAttribArray(0);
    ShaderLibrary::CompileShaders( pathStr + "/shaders" );

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void RenderDevice::ExecuteCommands()
{
    glClearDepth(1);
    glClearColor( 0, 0, 1, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


    std::for_each( m_commandBuffer.begin(), m_commandBuffer.end(), 
    []( ICommand* cmd )
    { 
        if (!cmd)
            return;

        cmd->Execute(); 
        delete cmd;
        cmd = nullptr;
    });

    m_commandBuffer.clear();
    glFlush();
}

void RenderDevice::Push( ICommand* cmd )
{
    m_commandBuffer.push_back(cmd);
}
