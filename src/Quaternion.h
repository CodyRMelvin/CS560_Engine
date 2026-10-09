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

#include <cmath>

class Quat
{
    public:

        union
        {
            struct
            {
                float w, x, y, z;
            };
            struct
            {
                float S;
                glm::vec3 V;
            };
            struct
            {
                glm::vec4 d;
            };
        };

        // Rule of three
        Quat() : d(0) {};
        Quat( float x, float y, float z, float w ) : d(w, x, y, z ) {}
        Quat( glm::vec4 d ) : d(d) {}
        Quat( float rads, glm::vec3 axis ) : S( std::cos( rads / 2.f ) ), V( std::sin( rads / 2.f ) * axis ) {}
        Quat( const Quat& other ) : V(other.V), S(other.S) {}
        Quat& operator=( const Quat& other );
        ~Quat() = default;

        // Operator overloads
        void operator+=( const Quat& other );
        void operator-=( const Quat& other );
        void operator*=( const Quat& other );
        void operator*=( float c );
        void operator/=( float c );

        // Non-editing mathematical functions
        float Dot( const Quat& other ) const;
        float MagnitudeSquared() const;
        float Magnitude() const;
        Quat Conjugate() const;
        Quat Inverse() const;
        glm::vec3 Rotate( const glm::vec3& v ) const;
        glm::mat4 RotationMatrix() const;

        // Editing mathematical functions
        void Conjugated();
        void Inverted();
        

};

inline Quat operator+ ( const Quat& lhs, const Quat& rhs )
{
    return Quat( glm::vec4( lhs.S + rhs.S, lhs.V + rhs.V ) );
}

inline Quat operator- ( const Quat& lhs, const Quat& rhs )
{
    return Quat( glm::vec4( lhs.S - rhs.S, lhs.V - rhs.V) );
}

inline Quat operator* ( const Quat& lhs, const Quat& rhs )
{
    return Quat( glm::vec4( lhs.S * rhs.S - glm::dot( lhs.V, rhs.V ) , lhs.S * rhs.V + rhs.S * lhs.V + glm::cross( lhs.V, rhs.V ) ) );
}

inline Quat operator* ( const Quat& lhs, float rhs )
{
    return Quat( glm::vec4( lhs.S * rhs, lhs.V * rhs) );
}

inline Quat operator/ ( const Quat& lhs, float rhs )
{
    return Quat( glm::vec4( lhs.S / rhs , lhs.V / rhs) );
}

inline float Dot( const Quat& lhs, const Quat& rhs )
{
    return glm::dot( lhs.V, rhs.V ) + lhs.S * rhs.S;
}

inline float MagnitudeSquared( const Quat& q )
{
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w + q.w;
}

inline float Magnitude( const Quat& q )
{
    return std::sqrt( MagnitudeSquared(q) );
}

inline Quat Conjugate( const Quat& q )
{
    return Quat( glm::vec4( q.S, -q.V ) );
}


inline Quat Invert( const Quat& q )
{
    float n = MagnitudeSquared(q);
    return Quat( glm::vec4( q.S / n, -q.V / n ) );
}

inline glm::vec3 Rotate( const Quat& q, const glm::vec3& v)
{
    return ( q * Quat( glm::vec4( 0, v ) ) * q.Conjugate() ).V;
}

inline glm::mat4 RotationMatrix( const Quat& quat )
{
    Quat q = quat / quat.Magnitude();
    return glm::mat4
    (
        1 - 2 * q.y * q.y - 2 * q.z * q.z, 2 * q.x * q.y - 2 * q.w * q.z,     2 * q.x * q.z + 2 * q.w * q.y,     0,
        2 * q.x * q.y + 2 * q.w * q.z,     1 - 2 * q.x * q.x - 2 * q.z * q.z, 2 * q.y * q.z - 2 * q.w * q.z,     0,
        2 * q.x * q.z - 2 * q.w * q.y,     2 * q.y * q.z + 2 * q.w * q.x,     1 - 2 * q.x * q.x - 2 * q.y * q.y, 0,
        0,                                 0,                                 0,                                 1
    );
}

#endif
