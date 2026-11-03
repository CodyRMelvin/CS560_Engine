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

        // Stores all the nodes in the scene with a safe index
        static std::unordered_map< uint64_t, Node*> NodeLibrary;

        // Constructs and virtual destructor to force virtual class
        Node() = default;
        Node( uint64_t id, uint64_t parent );
        Node( const Node& other );
        virtual ~Node() {}

        // Virtual methods responsible for primary operations of a node
        virtual void Initialize() = 0;
        virtual void Render() = 0;
        virtual void Destroy() = 0;

        // Getter for node ID
        uint64_t ID() const;

        // Getters and setters for node's parent ID
        bool HasParent() const;
        uint64_t ParentID() const;
        Node* Parent() const;
        void Parent( uint64_t parent );

        // Getters and setters for node's children
        uint64_t ChildID( size_t index ) const;
        Node* Child( size_t index ) const;
        std::vector<uint64_t> Children() const;
        void AddChild( uint64_t ID );

        // Give the parser direct acces to node's data
        friend class Parser;

    protected:
        // The index of this node within the library
        uint64_t m_ID;
        // The ID of the parent, will be equal to m_ID if this node has no parents 
        uint64_t m_parent;
        // Array of ID's for child nodes
        std::vector<uint64_t> m_children;
};

// Subclass of Node which forces the presence of a transform
class Node3D : public Node
{
    public:
        // Constructors and destructors
        Node3D() = default;
        Node3D( uint64_t id, uint64_t parent );
        Node3D( const Node3D& other );
        ~Node3D() {};

        // override primary methods to force non-virtual, empty implementation as they are not needed
        virtual void Initialize() override {}
        virtual void Render() override {}
        virtual void Destroy() override {}

        // Getters and setters for the Node's transform
        glm::mat4 GlobalTransformMatrix() const;
        glm::mat4 GlobalTransformMatrix();
        Transform LocalTransform() const;
        Transform& LocalTransform() { return m_transform; }
        void LocalTransform( const Transform& localTransform );

    protected:
        Transform m_transform;
};

// Helper function to delelte all nodes in the library
void DeleteAllNodes();

#endif
