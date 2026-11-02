//**************************************************************************************************
// File   : Math.h
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Public interface of variety of math utilities
//*************************************************************************************************

#ifndef MATH_H
#define MATH_H

#include "Quaternion.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fastgltf/math.hpp>

#include <cmath>

constexpr float epsilon = std::numeric_limits<float>::epsilon();

inline float Clamp( float x, float min, float max )
{
    return std::max( std::min( x, max ), min );
}

inline glm::mat4 AffineInverse(glm::mat4 M)
{
    glm::mat3 inverse( M );
    inverse = glm::inverse(inverse);

    glm::mat4 result(inverse);
    result[3][3] = 1;

    glm::vec3 invTranslate = -M[3];
    result = glm::translate( result, invTranslate );

    return result;
}

template< glm::length_t L, typename T, glm::qualifier Q >
inline glm::vec< L, T, Q > Lerp( glm::vec< L, T, Q > v1, glm::vec< L, T, Q > v2, float alpha )
{
    return ( ( 1 - alpha ) * v1 ) + ( alpha * v2 );
}

inline Quat SphericalLerp ( Quat q1, Quat q2, float alpha )
{
    q1.Normalize();
    q2.Normalize();

    float dot = glm::dot( q1.d, q2.d );
    // Force shortest path
    if ( dot < 0 )
    {
        q2 = -q2;
        dot = -dot;
    }

    dot = Clamp( dot, -1, 1 );
    // Nearly identical rotations can be lerped
    if ( dot > 0.9995 )
        return Normalize( q1 * ( 1 - alpha ) + q2 * alpha );

    float theta = std::acos(dot);
    float sinTheta = std::sin(theta);

    return q1 * ( std::sin( ( 1 - alpha ) * theta ) / sinTheta ) + 
           q2 * ( std::sin( alpha * theta ) / sinTheta );
}

#endif
