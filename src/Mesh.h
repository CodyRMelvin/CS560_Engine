//**************************************************************************************************
// File   : Mesh.h
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Public interface of class which encapsulates a 3D mesh
//**************************************************************************************************

#ifndef MESH_H
#define MESH_H

#include "Math.h"

#include <glad/glad.h>

#include <vector>

class Mesh
{
    public:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        Mesh( const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices );

    private:
        GLuint m_VAO, m_VBO, m_EBO;

        void InitializeMesh();
};

#endif
