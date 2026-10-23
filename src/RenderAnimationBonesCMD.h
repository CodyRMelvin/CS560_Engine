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
#include "Transform.h"
#include "Mesh.h"
#include "Joint.h"
#include "Application.h"

class RenderAnimationBonesCMD : ICommand
{
public:
    Pipeline pipeline;
    Transform globalTransform;
    uint64_t meshIndex;

    RenderAnimationBonesCMD() = default;
    ~RenderAnimationBonesCMD() = default;

    void Execute()
    {
        Mesh* mesh = static_cast<Mesh*>( Node::NodeLibrary[meshIndex] );
        if ( !mesh->HasSkin() )
        {
            Logger::LogLine( Logger::severityWarning, "Mesh {} passed into RenderAnimationBonesCMD has no skin", meshIndex );
            return;
        }

        Mesh::Skin skin = mesh->GetSkin();

        std::vector<glm::mat4> boneMats;
        boneMats.resize(skin.inverseBindMatrices.size());

        glm::mat4 inverseGlobal = glm::inverse( globalTransform.Matrix() );
        std::for_each(skin.joints.begin(), skin.joints.end(),
        [ &boneMats, &inverseGlobal, &skin ](uint64_t boneID)
        {
            Joint* j = dynamic_cast<Joint*>( Node::NodeLibrary[boneID] );
            if (!j)
                return;
            boneMats[j->matrixIndex] = inverseGlobal * j->GlobalTransformMatrix() * skin.inverseBindMatrices[j->matrixIndex];
        });

        pipeline.Bind();
        pipeline.ActiveShader(Shader::shaderTypeVertex);

        glm::mat4 modelView = Application::Get().GetActiveCamera().View() * globalTransform.Matrix();
        pipeline.UniformMat(0, modelView);
        pipeline.UniformMat(1, Application::Get().GetActiveCamera().Perspective());
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));
        pipeline.UniformMat(2, normalMatrix);
        pipeline.UniformMat(3, boneMats.data(), boneMats.size());

        pipeline.UnBind();
    }
};

#endif
