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
#include "Transform.h"

class RenderIndirectCMD : public ICommand
{
    public:
        uint64_t mesh;
        Pipeline pipeline;
        Transform transform;

        RenderIndirectCMD(){};
        ~RenderIndirectCMD(){}

        void Execute() override;

};


#endif
