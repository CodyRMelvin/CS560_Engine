//**************************************************************************************************
// File   : Node.cpp
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Implementation of interface representing node in scene heirarchy 
//**************************************************************************************************

#include "Node.h"
#include "Transform.h"
#include <sys/socket.h>

std::unordered_map< uint32_t, Node* > Node::NodeLibrary;

Node::Node( uint32_t id, uint32_t parent ) 
    : m_ID(id)
    , m_parent(parent)
    , m_children()
{ }

Node::Node( const Node& other )
    : m_ID(other.m_ID)
    , m_parent(other.m_parent)
    , m_children( other.m_children.begin(), other.m_children.end() )
{ }

uint32_t Node::ID() const
{
    return m_ID;
}

bool Node::HasParent() const
{
    return m_ID != m_parent;
}

uint32_t Node::ParentID() const
{
    return m_parent;
}

Node* Node::Parent() const
{
    return NodeLibrary[m_parent];
}

void Node::Parent( uint32_t parent )
{
    m_parent = parent;
}

uint32_t Node::ChildID( size_t index ) const
{
    return m_children[index];
}

Node* Node::Child( size_t index ) const
{
    return NodeLibrary[ m_children[index] ];
}

std::vector<uint32_t> Node::Children() const
{
    return m_children;
}

void Node::AddChild( uint32_t ID )
{
    m_children.push_back(ID);
}

Node3D::Node3D( uint32_t id, uint32_t parent )
    : Node( id, parent )
    , m_transform()
{ }

Node3D::Node3D( const Node3D& other )
    : Node(other)
    , m_transform(other.m_transform)
{ }

Transform Node3D::GlobalTransform() const
{
    if ( m_ID == m_parent )
        return m_transform;

    Node3D* parent =  dynamic_cast<Node3D*>( NodeLibrary[m_parent] );
    if (!parent)
        return m_transform;

    Transform pt = parent->GlobalTransform();
    Transform lt = m_transform;
    Transform global;
    global.Translation( lt.Translation() + pt.Translation() );
    global.Rotation( pt.Rotation() * lt.Rotation() );
    global.Scale( lt.Scale() + pt.Scale() );

    return global;
}

Transform Node3D::LocalTransform() const
{
    return m_transform;
}

void Node3D::LocalTransform( const Transform& localTransform )
{
    m_transform = localTransform;
}


void DeleteAllNodes()
{
    std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
    []( auto& pair )
    {
        pair.second->Destroy();
        delete pair.second;
        pair.second = nullptr;
    });

    Node::NodeLibrary.clear();
}
