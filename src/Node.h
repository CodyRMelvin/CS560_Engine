//**************************************************************************************************
// File   : Node.h
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Public interface of interface representing a node in scene heirarchy 
//**************************************************************************************************

#pragma once
#ifndef STUB_H
#define STUB_H

#include "Common.h"
#include "Transform.h"

#include <optional>

class Node
{
    public:

        static std::unordered_map< uint32_t, Node* > NodeLibrary;

        Node() = default;
        Node( uint32_t id, uint32_t parent );
        Node( const Node& other );
        virtual ~Node() {}

        virtual void Initialize() = 0;
        virtual void Render() = 0;
        virtual void Destroy() = 0;

        uint32_t ID() const;

        bool HasParent() const;
        uint32_t ParentID() const;
        Node* Parent() const;
        void Parent( uint32_t parent );

        uint32_t ChildID( size_t index ) const;
        Node* Child( size_t index ) const;
        std::vector<uint32_t> Children() const;
        void AddChild( uint32_t ID );

    protected:
        uint32_t m_ID;
        uint32_t m_parent;
        std::vector<uint32_t> m_children;
};

class Node3D : public Node
{
    public:
        Node3D() = default;
        Node3D( uint32_t id, uint32_t parent );
        Node3D( const Node3D& other );
        virtual ~Node3D() = 0;

        virtual void Initialize() = 0;
        virtual void Render() = 0;
        virtual void Destroy() = 0;

        Transform GlobalTransform() const;
        Transform LocalTransform() const;
        void LocalTransform( const Transform& localTransform );

    protected:
        Transform m_transform;
};

void DeleteAllNodes();

#endif
