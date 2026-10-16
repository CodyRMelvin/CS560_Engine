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
#include "Node.h"

#include <glad/glad.h>

typedef class Primitive Primitive;

class Mesh : public Node3D
{
    public:

        Mesh() : Node3D(), m_primitives(), m_drawBuffer(0) {}
        Mesh( uint32_t id, uint32_t parent );
        Mesh( size_t size ) : m_primitives(size), m_drawBuffer(0) {}
        Mesh( const Mesh& other );
        ~Mesh() = default;

        Mesh& operator=( const Mesh& other );

        void Initialize() override;
        void Render() override;
        void Destroy() override;

        void InitializeVertexArray( size_t index, GLenum primitiveType  );
        void InitializeVertexBuffer( size_t index, size_t count );
        void InitializeIndexBuffer( size_t index, size_t count, GLenum indexType );
        void InitializeDrawBuffer();
        void Destroy( size_t index );

        void* MapVertexBuffer( size_t index, GLenum access );
        void* MapIndexBuffer( size_t index, GLenum access );
        void UnMapVertexBuffer( size_t index );
        void UnMapIndexBuffer( size_t index );
        void EnableVertexAttribute( size_t primINdex, GLuint attribIndex, GLint size, GLenum type, GLuint offset );
        void EnableIndexBuffer( size_t index );

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
