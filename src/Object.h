//**************************************************************************************************
// File   : Object.h
// Author : Cody Melvin
// Date   : 2026/09/19
// Brief  : Interface for class encapsulating 3D object data 
//**************************************************************************************************

#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#include "Mesh.h"
#include "Transform.h"

#include <filesystem>

class Object
{
    public:

        Mesh mesh;
        Transform transform;

        Object() = default;
        Object( Mesh m, Transform t );
        ~Object() = default;

        void Destroy();
        // Stubs for later use
        void Render( GLuint program );
        void Deserialize(std::filesystem::directory_entry source);
};

#endif
