//**************************************************************************************************
// File   : Mesh.h
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Public interface of class which encapsulates a 3D mesh
//**************************************************************************************************

#pragma once
#ifndef MESH_H
#define MESH_H

#include "Common.h"
#include "Transform.h"

#include <glad/glad.h>

typedef class Primitive Primitive;

class Mesh
{
    public:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        Mesh() = default;
        Mesh( const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices );
        ~Mesh() = default;

        void Draw( Transform& transform ) const;
        void Destroy();

    private:
        GLuint m_VAO, m_VBO, m_EBO;

        void InitializeMesh();
};

class Mesh2
{
    public:

        Mesh2();
        ~Mesh2();

        void Initialize();

    private:
        struct Primitive
        {
            GLuint VAO, EBO, VBO;
            GLenum type;
        };

        std::vector<Primitive> m_primitves;

};

#endif
