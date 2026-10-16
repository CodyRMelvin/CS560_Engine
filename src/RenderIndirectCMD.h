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
#include "Mesh.h"
#include "Pipeline.h"
#include "Transform.h"

class RenderIndirectCMD : public ICommand
{
    public:
        Mesh mesh;
        Pipeline pipeline;
        Transform transform;

        RenderIndirectCMD(){};
        ~RenderIndirectCMD(){}

        void Execute() override;

};


#endif
