//**************************************************************************************************
// File   : Quaternion.cpp
// Author : Cody Melvin
// Date   : 2026/10/2
// Brief  : Implementation of non-inline quaternion class methods
//**************************************************************************************************

#include "Quaternion.h"

Quat& Quat::operator=( const Quat& other )
{
    if ( this == &other )
        return *this;

    this->d = other.d;
    return *this;
}

void Quat::operator+=( const Quat& other )
{
    S += other.S;
    V += other.V;
}

void Quat::operator-=( const Quat& other )
{
    S -= other.S;
    V -= other.V;
}

void Quat::operator*=( const Quat& other )
{
    float newS = S * other.S - glm::dot( V, other.V );
    V = S * other.V + other.S * V + glm::cross( V, other.V );
    S = newS;
}

void Quat::operator*=( float c )
{
    S *= c;
    V *= c;
}

void Quat::operator/=( float c )
{
    S /= c;
    V /= c;
}

float Quat::Dot( const Quat& other ) const
{
    return glm::dot( V, other.V ) + S * other.S;
}

float Quat::MagnitudeSquared() const
{
    return x * x + y * y + z * z + w * w;
}

float Quat::Magnitude() const
{
    return std::sqrt( MagnitudeSquared() );
}

Quat Quat::Conjugate() const
{
    return Quat( glm::vec4( S, -V ) );
}

Quat Quat::Inverse() const
{
    float n = MagnitudeSquared();
    return Quat( glm::vec4( S / n,  -V / n)  );
}

glm::vec3 Quat::Rotate( const glm::vec3& v ) const
{
    Quat qv( glm::vec4( 0, v ) );
    return ( *this * qv * this->Conjugate() ).V;
}

glm::mat4 Quat::RotationMatrix() const
{
    float mag = this->Magnitude();
    if (!mag)
        return glm::mat4(1);

    Quat q = *this / mag;
    return glm::mat4
    (
        1 - 2 * q.y * q.y - 2 * q.z * q.z, 2 * q.x * q.y - 2 * q.w * q.z,     2 * q.x * q.z + 2 * q.w * q.y,     0,
        2 * q.x * q.y + 2 * q.w * q.z,     1 - 2 * q.x * q.x - 2 * q.z * q.z, 2 * q.y * q.z - 2 * q.w * q.z,     0,
        2 * q.x * q.z - 2 * q.w * q.y,     2 * q.y * q.z + 2 * q.w * q.x,     1 - 2 * q.x * q.x - 2 * q.y * q.y, 0,
        0,                                 0,                                 0,                                 1
    );
}

void Quat::Conjugated()
{
    V *= -1;
}

void Quat::Inverted()
{
    float n = MagnitudeSquared();
    S /= n;
    V /= -n;
}

