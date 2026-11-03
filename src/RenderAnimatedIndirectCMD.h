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

        // Override execute method to implement execution of this command
        void Execute() override
        {

            // Get the mesh, if no mesh is present at given index, log an error and exit;
            Mesh* mesh = static_cast<Mesh*>( Node::NodeLibrary[meshID] );
            if ( !mesh->HasSkin() )
            {
                Logger::LogLine( Logger::severityError, "Mesh {} passed to RenderAnimatedIndirectCMD has no skin", meshID );
                return;
            }

            Mesh::Skin skin = mesh->GetSkin();

            // Create array to store all bone transformation
            std::vector<glm::mat4> boneMats;
            boneMats.resize( skin.inverseBindMatrices.size() );
            // Get inverse matrix of mesh's global transform
            glm::mat4 inverseGlobal = glm::inverse( mesh->GlobalTransformMatrix() );

            // For each joint in the skin, calculate it's transform matrix
            for( int i = 0;  i < skin.joints.size(); ++i )
            {
                Node3D* j = dynamic_cast<Node3D*>( Node::NodeLibrary[ skin.joints[i] ] );
                if (!j)
                    return;

                // bone matrix is the product of the mesh's transform invers, global matrix of the joint, and the joint's inverse bind matrix
                boneMats[i] = inverseGlobal * j->GlobalTransformMatrix() * skin.inverseBindMatrices[i];
            }

            // Bind the given pipeline and set it's vertex shader active
            pipeline.Bind();
            pipeline.ActiveShader(Shader::shaderTypeVertex);

            // Pass matrices to the vertex shader
            glm::mat4 modelView = Application::Get().GetActiveCamera().View() * mesh->GlobalTransformMatrix();
            pipeline.UniformMat( 0, modelView ); 
            pipeline.UniformMat( 1, Application::Get().GetActiveCamera().Perspective() );
            glm::mat4 normalMatrix = glm::transpose( glm::inverse(modelView) );
            pipeline.UniformMat( 2, normalMatrix );
            pipeline.UniformMat( 3, boneMats.data(), boneMats.size() );

            // Draw the mesh's primitives and unbind the pipeline
            mesh->DrawPrimitives();
            pipeline.UnBind();
        }

};

#endif
