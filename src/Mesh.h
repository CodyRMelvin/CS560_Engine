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

class MeshOld
{
    public:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        MeshOld() = default;
        MeshOld( const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices );
        ~MeshOld() = default;

        void Draw( Transform& transform ) const;
        void Destroy();

    private:
        GLuint m_VAO, m_VBO, m_EBO;

        void InitializeMeshOld();
};

class Mesh
{
    public:

        Mesh() : m_primitives(), m_drawBuffer(0) {}
        Mesh( size_t size ) : m_primitives(size), m_drawBuffer(0) {}
        Mesh( const Mesh& other );
        ~Mesh() = default;

        Mesh& operator=( const Mesh& other );

        void InitializeVertexArray( size_t index, GLenum primitiveType  );
        void InitializeVertexBuffer( size_t index, size_t count );
        void InitializeIndexBuffer( size_t index, size_t count, GLenum indexType );
        void InitializeDrawBuffer();
        void Destroy( size_t index );
        void DestroyAll();

        void* MapVertexBuffer( size_t index, GLenum access );
        void* MapIndexBuffer( size_t index, GLenum access );
        void UnMapVertexBuffer( size_t index );
        void UnMapIndexBuffer( size_t index );
        void EnableVertexAttribute( size_t primINdex, GLuint attribIndex, GLint size, GLenum type, GLuint offset );
        void EnableIndexBuffer( size_t index );

        void Draw( Transform& t ) const;

    private:
        struct IndirectDrawCommand
        {
            GLuint vertexCount;
            GLuint instanceCount;
            GLuint firstIndex;
            GLint baseVertex;
            GLuint baseInstance;
        };
        struct Primitive
        {
            IndirectDrawCommand cmd;
            GLuint VAO, EBO, VBO;
            GLenum primType, indexType;
        };

        std::vector<Primitive> m_primitives;
        GLuint m_drawBuffer;

};

#endif
