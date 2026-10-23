//**************************************************************************************************
// File   : RenderIndirectCMD.cpp
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : 
//**************************************************************************************************

#include "RenderIndirectCMD.h"
#include "Application.h"
#include "Mesh.h"

void RenderIndirectCMD::Execute()
{
    pipeline.Bind();
    pipeline.ActiveShader(Shader::shaderTypeVertex);

    glm::mat4 modelView = Application::Get().GetActiveCamera().View() * transform;
    pipeline.UniformMat( 0, modelView ); 
    pipeline.UniformMat( 1, Application::Get().GetActiveCamera().Perspective() );
    glm::mat4 normalMatrix = glm::transpose( glm::inverse(modelView) );
    pipeline.UniformMat( 2, normalMatrix );

    static_cast<Mesh*>( Node::NodeLibrary[mesh] )->DrawPrimitives();

    pipeline.UnBind();
}
