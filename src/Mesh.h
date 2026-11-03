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

// Forward reference
typedef class Primitive Primitive;

// Struct to hold vertex data 
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
        // Struct to hold skin data
        struct Skin
        {
            std::vector<glm::mat4> inverseBindMatrices;
            std::vector<uint64_t> joints;
            uint64_t skeleton;
        };

        // Rule of three
        Mesh() : Node3D(), m_primitives(), m_drawBuffer(0) {}
        Mesh( uint32_t id, uint32_t parent );
        Mesh( size_t size ) : m_primitives(size), m_drawBuffer(0) {}
        Mesh( const Mesh& other );
        Mesh& operator=( const Mesh& other );
        ~Mesh() = default;

        // Override virtual methods from Node3D
        virtual void Initialize() override;
        virtual void Render() override;
        virtual void Destroy() override;

        // Methods to create VAOs and buffers in primitives
        void InitializeVertexArray( size_t index, GLenum primitiveType );
        void InitializeVertexBuffer( size_t index, size_t count, size_t vertexSize = sizeof(Vertex)  );
        void InitializeIndexBuffer( size_t index, size_t count, GLenum indexType );
        // Method to create the draw buffer
        void InitializeDrawBuffer();
        // Getter for animation primitive's index
        size_t GetAnimationPrimitive();
        // Destroys primitive at the given index
        void Destroy( size_t index );

        // Map gpu memory of vertex and index buffers to the cpu
        void* MapVertexBuffer( size_t index, GLenum access );
        void* MapIndexBuffer( size_t index, GLenum access );
        // Return buffer memory to gpu
        void UnMapVertexBuffer( size_t index );
        void UnMapIndexBuffer( size_t index );
        // Enable vertex attributes at a given primitive
        void EnableVertexAttribute( size_t primINdex, GLuint attribIndex, GLint size, GLenum type, GLuint offset, size_t vertexSize = sizeof(Vertex) );
        void EnableVertexAttributeI( size_t primIndex, GLuint attribIndex, GLint size, GLenum type, GLuint offset, size_t vertexSize = sizeof(Vertex) );
        // Bind the index buffer to the va
        void EnableIndexBuffer( size_t index );

        // Methods to draw the primitives
        void DrawPrimitives() const;
        void DrawAnimationPrimitive() const;
        void DrawPrimitive( size_t index ) const;

        // Getter/Setter for skin
        bool HasSkin() const { return m_skin.has_value(); }
        Skin GetSkin() const { return m_skin.value(); }
        void SetSkin( const Skin& skin ) { m_skin = skin; }

        // Getter setter for the animation primitive index
        // TODO: adjust redundancy
        bool HasAnimationPrimitive() const { return m_animationPrimitiveIndex.has_value(); }
        bool GetAnimationPrimitive() const { return m_animationPrimitiveIndex.value(); }

    protected:
        // Information for the draw command for indirect rendering
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
        // Information stored in a primitive
        struct Primitive
        {
            IndirectDrawCommand cmd;
            GLuint VAO, EBO, VBO;
            GLenum primType, indexType;
        };

        // Array of primitives
        std::vector<Primitive> m_primitives;
        // skin and animation primitive index are not required
        std::optional<Skin> m_skin;
        std::optional<size_t> m_animationPrimitiveIndex;
};

#endif
