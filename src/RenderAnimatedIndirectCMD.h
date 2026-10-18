//**************************************************************************************************
// File   : RenderAnimatedIndirectCMD.h
// Author : Cody Melvin
// Date   : 2026/xx/xx
// Brief  : 
//**************************************************************************************************

#pragma once
#ifndef RENDERANIMATEDINDIRECTCMD_H
#define RENDERANIMATEDINDIRECTCMD_H

#include "Common.h"
#include "RenderDevice.h"
#include "Pipeline.h"
#include "Transform.h"
#include "Node.h"
#include "Application.h"
#include "Mesh.h"
#include "Joint.h"
#include "Transform.h"

class RenderAnimatedIndirectCMD: public ICommand
{
    public:
        uint64_t meshID;
        Pipeline pipeline;
        Transform transform;

        RenderAnimatedIndirectCMD() = default;
        ~RenderAnimatedIndirectCMD() = default;

        void Execute() override
        {

            Mesh* mesh = static_cast<Mesh*>( Node::NodeLibrary[meshID] );
            if ( !mesh->HasSkin() )
            {
                Logger::LogLine( Logger::severityError, "Mesh {} passed to RenderAnimatedIndirectCMD has no skin", meshID );
                return;
            }

            Mesh::Skin skin = mesh->GetSkin();

            std::vector<glm::mat4> boneMats( skin.inverseBindMatrices.begin(), skin.inverseBindMatrices.end() );
            std::vector<uint64_t> jointIDs;
            jointIDs.push_back(skin.skeleton);

            // Recursive lambda to populate jointIDs 
            std::function< void(uint64_t) > GetJoints = [ &jointIDs, &GetJoints ](uint64_t index)
            {
                std::vector<uint64_t> children = Node::NodeLibrary[index]->Children();
                std::for_each( children.begin(), children.end(), GetJoints );
            };
            GetJoints( jointIDs.front() );

            std::for_each( jointIDs.begin(), jointIDs.end(),
            [&boneMats]( uint64_t boneID )
            {
                Joint* j = static_cast<Joint*>( Node::NodeLibrary[boneID] );
                boneMats[j->matrixIndex] = boneMats[j->matrixIndex] * j->GlobalTransform().Matrix();
            }); 

            pipeline.Bind();
            pipeline.ActiveShader(Shader::shaderTypeVertex);

            glm::mat4 modelView = Application::Get().GetActiveCamera().View() * transform.Matrix();
            pipeline.UniformMat( 0, modelView ); 
            pipeline.UniformMat( 1, Application::Get().GetActiveCamera().Perspective() );
            glm::mat4 normalMatrix = glm::transpose( glm::inverse(modelView) );
            pipeline.UniformMat( 2, normalMatrix );
            pipeline.UniformMat( 3, skin.inverseBindMatrices.data(), skin.inverseBindMatrices.size() );

            mesh->DrawPrimitives();

            pipeline.UnBind();
        }

};

#endif
