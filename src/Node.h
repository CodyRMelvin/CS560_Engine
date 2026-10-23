//**************************************************************************************************
// File   : Node.h
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Public interface of interface representing a node in scene heirarchy 
//**************************************************************************************************

#pragma once
#include <unordered_map>
#ifndef STUB_H
#define STUB_H

#include "Common.h"
#include "Transform.h"

//typedef class Parser Parser;

class Node
{
    public:

        static std::unordered_map< uint64_t, Node*> NodeLibrary;

        Node() = default;
        Node( uint64_t id, uint64_t parent );
        Node( const Node& other );
        virtual ~Node() {}

        virtual void Initialize() = 0;
        virtual void Render() = 0;
        virtual void Destroy() = 0;

        uint64_t ID() const;

        bool HasParent() const;
        uint64_t ParentID() const;
        Node* Parent() const;
        void Parent( uint64_t parent );

        uint64_t ChildID( size_t index ) const;
        Node* Child( size_t index ) const;
        std::vector<uint64_t> Children() const;
        void AddChild( uint64_t ID );

        friend class Parser;

    protected:
        uint64_t m_ID;
        uint64_t m_parent;
        std::vector<uint64_t> m_children;
};

class Node3D : public Node
{
    public:
        Node3D() = default;
        Node3D( uint64_t id, uint64_t parent );
        Node3D( const Node3D& other );
        virtual ~Node3D() {};

        virtual void Initialize() = 0;
        virtual void Render() = 0;
        virtual void Destroy() = 0;

        //Transform GlobalTransform() const;
        glm::mat4 GlobalTransformMatrix() const;
        //Transform GlobalTransform();
        glm::mat4 GlobalTransformMatrix();
        Transform LocalTransform() const;
        Transform& LocalTransform() { return m_transform; }
        void LocalTransform( const Transform& localTransform );

    protected:
        Transform m_transform;
};

void DeleteAllNodes();

#endif
