//**************************************************************************************************
// File   : Joint.h
// Author : Cody Melvin
// Date   : 2026/10/16
// Brief  : Node3D subclass for joints
//**************************************************************************************************

#pragma once
#ifndef JOINT_H
#define JOINT_H

#include "Node.h"

class Joint : public Node3D
{
    public:
        uint64_t matrixIndex;

        Joint() = default;
        Joint( uint64_t id, uint64_t parent ) : Node3D( id, parent ), matrixIndex(0) {}
        Joint( const Joint& other ) : Node3D(other), matrixIndex(other.matrixIndex) {}
        ~Joint() {};

        void Initialize() {}
        void Render() {}
        void Destroy() {}
};


#endif
