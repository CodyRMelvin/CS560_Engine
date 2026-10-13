//**************************************************************************************************
// File   : Stub.h
// Author : Cody Melvin
// Date   : 2026/09/19
// Brief  : Implementation for class encapsulating 3D object data 
//**************************************************************************************************

#include "Shader.h"
#define UNUSED(x) (void)(x)
#include "Object.h"

Object::Object( Mesh m, Transform t )
    : mesh(m)
    , transform(t)
{ }

void Object::Destroy()
{
    mesh.DestroyAll();
}

void Object::Render( GLuint program )
{
    mesh.Draw(transform);
}

void Object::Deserialize(std::filesystem::directory_entry source)
{
    UNUSED(source);
}
