//**************************************************************************************************
// File   : RenderAnimationBonesCMD.h
// Author : Cody Melvin
// Date   : 2026/10/20
// Brief  : Render command for rending the bones of an animation
//**************************************************************************************************

#pragma once
#ifndef RENDERANIMATIONBONES_H
#define RENDERANIMATIONBONES_H

#include "Common.h"
#include "RenderDevice.h"
#include "Pipeline.h"
#include "Mesh.h"
#include "Application.h"
#include "Joint.h"

class RenderAnimationBonesCMD : public ICommand
{
public:
    Pipeline pipeline;
    glm::mat4 transform;
    uint64_t meshIndex;

    RenderAnimationBonesCMD() = default;
    ~RenderAnimationBonesCMD() = default;

    void Execute()
    {
        pipeline.Bind();
        pipeline.ActiveShader(Shader::shaderTypeVertex);

        glm::mat4 modelView = Application::Get().GetActiveCamera().View() * transform;
        pipeline.UniformMat( 0, modelView ); 
        pipeline.UniformMat( 1, Application::Get().GetActiveCamera().Perspective() );
        glm::mat4 normalMatrix = glm::transpose( glm::inverse(modelView) );
        pipeline.UniformMat( 2, normalMatrix );

        Mesh* m = static_cast<Mesh*>( Node::NodeLibrary[meshIndex] );
        m->DrawAnimationPrimitive();

        // Rewriting the position values of the skeleton to reflect the cpu side bones
        // Absolutely not how I should be using indirect rendering but I'm in too deep now
            // Possibly implement a dynamic, direct mesh class to handle this correctly down the line
        Vertex* vertices = static_cast<Vertex*>( m->MapVertexBuffer( m->GetAnimationPrimitive(), GL_WRITE_ONLY ) );
        for ( int i = 0; i < m->GetSkin().joints.size(); ++i )
        {
            vertices[i].pos = static_cast<Joint*>( Node::NodeLibrary[ m->GetSkin().joints[i] ] )->GlobalTransformMatrix()[3];
            //vertices[i].pos = static_cast<Joint*>( Node::NodeLibrary[ m->GetSkin().joints[i] ] )->LocalTransform().Matrix()[3];
        }
        m->UnMapVertexBuffer( m->GetAnimationPrimitive() );

        pipeline.UnBind();
    }
};

#endif
