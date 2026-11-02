//**************************************************************************************************
// File   : RenderAnimatedIndirectCMD.h
// Author : Cody Melvin
// Date   : 2026/10/17
// Brief  : Render command for animated meshes
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
#include "Transform.h"

class RenderAnimatedIndirectCMD: public ICommand
{
    public:
        uint64_t meshID;
        Pipeline pipeline;

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

            std::vector<glm::mat4> boneMats;
            boneMats.resize( skin.inverseBindMatrices.size() );
            glm::mat4 inverseGlobal = glm::inverse( mesh->GlobalTransformMatrix() );

            for( int i = 0;  i < skin.joints.size(); ++i )
            {
                Node3D* j = dynamic_cast<Node3D*>( Node::NodeLibrary[ skin.joints[i] ] );
                if (!j)
                    return;

                boneMats[i] = inverseGlobal * j->GlobalTransformMatrix() * skin.inverseBindMatrices[i];
            }

            pipeline.Bind();
            pipeline.ActiveShader(Shader::shaderTypeVertex);

            glm::mat4 modelView = Application::Get().GetActiveCamera().View() * mesh->GlobalTransformMatrix();
            pipeline.UniformMat( 0, modelView ); 
            pipeline.UniformMat( 1, Application::Get().GetActiveCamera().Perspective() );
            glm::mat4 normalMatrix = glm::transpose( glm::inverse(modelView) );
            pipeline.UniformMat( 2, normalMatrix );
            pipeline.UniformMat( 3, boneMats.data(), boneMats.size() );

            mesh->DrawPrimitives();

            pipeline.UnBind();
        }

};

#endif
