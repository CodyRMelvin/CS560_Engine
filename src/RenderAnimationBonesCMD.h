//**************************************************************************************************
// File   : RenderAnimationBonesCMD.h
// Author : Cody Melvin
// Date   : 2026/10/20
// Brief  : Render command for rending the bones of an animation
//**************************************************************************************************

#pragma once
#ifndef STUB_H
#define STUB_H

#include "RenderDevice.h"
#include "Pipeline.h"
#include "Transform.h"
#include "Mesh.h"
#include "Joint.h"
#include "Application.h"

#include "glm/glm.hpp"

class RenderAnimationBonesCMD : ICommand
{
public:
    Pipeline pipeline;
    Transform globalTransform;
    Mesh::Skin skin;

    RenderAnimationBonesCMD() = default;
    ~RenderAnimationBonesCMD() = default;
    void Execute()
    {
        std::vector<glm::mat4> boneMats;
        boneMats.resize(skin.inverseBindMatrices.size());

        glm::mat4 inverseGlobal = glm::inverse( globalTransform.Matrix() );
        std::for_each(skin.joints.begin(), skin.joints.end(),
        [ &boneMats, &inverseGlobal, this ](uint64_t boneID)
        {
            Joint* j = dynamic_cast<Joint*>( Node::NodeLibrary[boneID] );
            if (!j)
                return;
            boneMats[j->matrixIndex] = inverseGlobal * j->GlobalTransformMatrix() * this->skin.inverseBindMatrices[j->matrixIndex];
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
