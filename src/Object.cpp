//**************************************************************************************************
// File   : Stub.h
// Author : Cody Melvin
// Date   : 2026/09/19
// Brief  : Implementation for class encapsulating 3D object data 
//**************************************************************************************************

#include "Object.h"
#include "RenderDevice.h"
#include "RenderIndirectCMD.h"

#define UNUSED(x) (void)(x)

Object::Object( Mesh m, Transform t )
    : mesh(m)
    , transform(t)
{ }

void Object::Destroy()
{
    mesh.Destroy();
    pipeline.Destroy();
}

void Object::Render()
{
    RenderIndirectCMD* cmd = new RenderIndirectCMD();
    cmd->mesh = mesh;
    cmd->transform = transform;
    cmd->pipeline = pipeline;

    RenderDevice::Get().Push(cmd); 
}

void Object::Deserialize(std::filesystem::directory_entry source)
{
    UNUSED(source);
}
