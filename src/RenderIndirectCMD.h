//**************************************************************************************************
// File   : RenderIndirectCMD.h
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : ICommand subclass for indirect rendering
//**************************************************************************************************

#pragma once
#ifndef RENDERINDIRECTCMD_H
#define RENDERINDIRECTCMD_H

#include "Common.h"
#include "RenderDevice.h"
#include "Pipeline.h"
#include "Application.h"
#include "Mesh.h"

class RenderIndirectCMD : public ICommand
{
    public:
        uint64_t mesh;
        Pipeline pipeline;
        glm::mat4 transform;

        RenderIndirectCMD(){};
        ~RenderIndirectCMD(){}

        void Execute() override
        {
            // Bind the pipeline and set the active shader to the vertex shader
            pipeline.Bind();
            pipeline.ActiveShader(Shader::shaderTypeVertex);

            // Pass in matrices
            glm::mat4 modelView = Application::Get().GetActiveCamera().View() * transform;
            pipeline.UniformMat( 0, modelView ); 
            pipeline.UniformMat( 1, Application::Get().GetActiveCamera().Perspective() );
            glm::mat4 normalMatrix = glm::transpose( glm::inverse(modelView) );
            pipeline.UniformMat( 2, normalMatrix );

            // Get the mesh and draw primitives
            static_cast<Mesh*>( Node::NodeLibrary[mesh] )->DrawPrimitives();
            // Unbind the pipeline
            pipeline.UnBind();
        }
};


#endif
