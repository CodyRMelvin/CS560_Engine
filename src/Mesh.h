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
#include "Pipeline.h"

#include <glad/glad.h>

typedef class Primitive Primitive;

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 nrm;
    glm::vec2 tex;

    glm::ivec4 boneIds;
    glm::vec4 weights;
};

class Mesh : public Node3D
{
    public:
        static const size_t MAX_BONE_INFLUENCE = 4;
        struct Skin
        {
            std::vector<glm::mat4> inverseBindMatrices;
            std::vector<uint64_t> joints;
            uint64_t skeleton;
        };

        Mesh() : Node3D(), m_primitives(), m_drawBuffer(0) {}
        Mesh( uint32_t id, uint32_t parent );
        Mesh( size_t size ) : m_primitives(size), m_drawBuffer(0) {}
        Mesh( const Mesh& other );
        ~Mesh() = default;

        Mesh& operator=( const Mesh& other );

        virtual void Initialize() override;
        virtual void Render() override;
        virtual void Destroy() override;

        virtual void InitializeVertexArray( size_t index, GLenum primitiveType );
        virtual void InitializeVertexBuffer( size_t index, size_t count, size_t vertexSize = sizeof(Vertex)  );
        virtual void InitializeIndexBuffer( size_t index, size_t count, GLenum indexType );
        virtual void InitializeDrawBuffer();
        virtual void Destroy( size_t index );

        virtual void* MapVertexBuffer( size_t index, GLenum access );
        virtual void* MapIndexBuffer( size_t index, GLenum access );
        virtual void UnMapVertexBuffer( size_t index );
        virtual void UnMapIndexBuffer( size_t index );
        virtual void EnableVertexAttribute( size_t primINdex, GLuint attribIndex, GLint size, GLenum type, GLuint offset, size_t vertexSize = sizeof(Vertex) );
        virtual void EnableVertexAttributeI( size_t primIndex, GLuint attribIndex, GLint size, GLenum type, GLuint offset, size_t vertexSize = sizeof(Vertex) );
        virtual void EnableIndexBuffer( size_t index );

        virtual void DrawPrimitives() const;

        bool HasSkin() const { return m_skin.has_value(); }
        Skin GetSkin() const { return m_skin.value(); }
        void SetSkin( const Skin& skin ) { m_skin = skin; }

    protected:
        struct IndirectDrawCommand
        {
            GLuint vertexCount;
            GLuint instanceCount;
            GLuint firstIndex;
            GLint baseVertex;
            GLuint baseInstance;
        };
        GLuint m_drawBuffer;
        Pipeline m_pipeline;

    private:
        struct Primitive
        {
            IndirectDrawCommand cmd;
            GLuint VAO, EBO, VBO;
            GLenum primType, indexType;
        };

        std::vector<Primitive> m_primitives;
        std::optional<Skin> m_skin;
};

#endif
