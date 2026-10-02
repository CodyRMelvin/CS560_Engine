//**************************************************************************************************
// File   : Quaternion.h
// Author : Cody Melvin
// Date   : 2026/10/2
// Brief  : Public interface and inline implementations of class representing a quaternion
//**************************************************************************************************

#pragma once
#ifndef QUATERNION_H
#define QUATERNION_H

#include <glm/glm.hpp>

class Quat
{
    public:

        union
        {
            struct
            {
                float x, y, z, w;
            };
            struct
            {
                glm::vec3 V;
                float S;
            };
            struct
            {
                float d[4];
            };
        };

        // Rule of three
        Quat();
        Quat( float x, float y, float z, float w ) : V( x, y, z ), S(w) {}
        Quat( glm::vec3 V, float S ) : V(V), S(S) {}
        Quat( const Quat& other ) : V(other.V), S(other.S) {}
        void operator=( const Quat& other ){ *this = Quat(other); }
        ~Quat() = default;

    private:
};

#endif
