//**************************************************************************************************
// File   : Node.cpp
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Implementation of interface representing node in scene heirarchy 
//**************************************************************************************************

#include "Node.h"
#include "Transform.h"
#include "Animation.h"

std::unordered_map< uint64_t, Node* > Node::NodeLibrary;

Node::Node( uint64_t id, uint64_t parent ) 
    : m_ID(id)
    , m_parent(parent)
    , m_children()
{ }

Node::Node( const Node& other )
    : m_ID(other.m_ID)
    , m_parent(other.m_parent)
    , m_children( other.m_children.begin(), other.m_children.end() )
{ }

uint64_t Node::ID() const
{
    return m_ID;
}

bool Node::HasParent() const
{
    return m_ID != m_parent;
}

uint64_t Node::ParentID() const
{
    return m_parent;
}

Node* Node::Parent() const
{
    return NodeLibrary[m_parent];
}

void Node::Parent( uint64_t parent )
{
    m_parent = parent;
}

uint64_t Node::ChildID( size_t index ) const
{
    return m_children[index];
}

Node* Node::Child( size_t index ) const
{
    return NodeLibrary[ m_children[index] ];
}

std::vector<uint64_t> Node::Children() const
{
    return m_children;
}

void Node::AddChild( uint64_t ID )
{
    m_children.push_back(ID);
}

Node3D::Node3D( uint64_t id, uint64_t parent )
    : Node( id, parent )
    , m_transform()
{ }

Node3D::Node3D( const Node3D& other )
    : Node(other)
    , m_transform(other.m_transform)
{ }

glm::mat4 Node3D::GlobalTransformMatrix() const
{
    // Recursive base case, if there is no parent than the local transform is this node's global transform
    if (m_ID == m_parent)
        return m_transform.Matrix();

    // If no parent with a transform is present, return the local transform
    Node3D* parent = dynamic_cast<Node3D*>(NodeLibrary[m_parent]);
    if (!parent)
        return m_transform.Matrix();
    // Multiply recur to get paren't global transform then multiply with local transform
    glm::mat4 result = parent->GlobalTransformMatrix() * m_transform.Matrix();
    return result;
}

glm::mat4 Node3D::GlobalTransformMatrix()
{
    // Same operations as prior but without constness
    if (m_ID == m_parent)
        return m_transform.Matrix();

    Node3D* parent = dynamic_cast<Node3D*>(NodeLibrary[m_parent]);
    if (!parent)
        return m_transform.Matrix();

    glm::mat4 result = parent->GlobalTransformMatrix() * m_transform.Matrix();
    return result;
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
    // Iterate through node library and delete each node
    std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
    []( auto& pair )
    {
        delete pair.second;
        pair.second = nullptr;
    });

    // Empty the node library
    Node::NodeLibrary.clear();
    // Empty the animation library
    // TODO: find better place/way to manage this
    Animation::AnimationLibrary.clear();
}
