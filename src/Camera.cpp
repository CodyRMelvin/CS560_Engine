//**************************************************************************************************
// File   : Camera.h
// Author : Cody Melvin
// Date   : 2025/07/18
// Brief  : Implementation file for class containing camera information and operations
//**************************************************************************************************

#include "Camera.h"

#include "Common.h"

#include <glm/ext/matrix_transform.hpp>

Camera::Camera()
    : m_center( { 0, 0, 0 } )
    , m_right( { 1, 0, 0 } )
    , m_back( { 0, 0, 1 } )
    , m_up( { 0, 1, 0 } )
    , m_vpDistance(500.05)
    , m_nearDistance(.1)
    , m_farDistance(1000)
    , m_vpWidth(10.1)
    , m_vpHeight(10.1)
{ }

Camera::Camera(const Camera& other)
    : m_center(other.m_center)
    , m_right(other.m_right)
    , m_back(other.m_back)
    , m_up(other.m_up)
    , m_vpDistance(other.m_vpDistance)
    , m_nearDistance(other.m_nearDistance)
    , m_farDistance(other.m_farDistance)
    , m_vpWidth(other.m_vpWidth)
    , m_vpHeight(other.m_vpHeight)
{ }

void Camera::operator=(const Camera& other)
{
    *this = Camera(other);
}

Camera& Camera::SetViewingDimensions( float fov, float aspectRatio )
{
    m_vpWidth = 2 * m_vpDistance * std::tan( glm::radians(fov) * .5 );
    m_vpHeight = m_vpWidth / aspectRatio;
    return *this;
}

Camera& Camera::SetPlaneDistances( float near, float far )
{
    float fov = std::atan( m_vpWidth / (2 * m_vpDistance) ) / 2;
    float aspectRatio = m_vpWidth / m_vpHeight;

    m_nearDistance = near;
    m_farDistance = far;
    m_vpDistance = ( far - near ) / 2 + near;

    SetViewingDimensions( glm::degrees(fov), aspectRatio );

    return *this;
}

Camera& Camera::LookAt( glm::vec3 position, glm::vec3 lookAt, glm::vec3 up )
{
    glm::mat4 m = glm::lookAt( position, lookAt - position, up );
    m_center = position;
    m_right = m[0];
    m_up = m[1];
    m_back = m[2];
    return *this;
}

glm::vec3 Camera::Origin() const
{
    return m_center;
}

glm::vec3 Camera::Up() const
{
    return m_up;
}

glm::vec3 Camera::Right() const
{
    return m_right;
}

glm::vec3 Camera::Back() const
{
    return m_back;
}

glm::vec3 Camera::Viewport() const
{
    return glm::vec3( m_vpWidth, m_vpHeight, m_vpDistance );
}

float Camera::Near() const
{
    return m_nearDistance;
}

float Camera::Far() const
{
    return m_farDistance;
}

Camera& Camera::Zoom(float factor)
{
    m_vpWidth *= factor;
    m_vpHeight *= factor;
    return *this;
}

Camera& Camera::Forwards(float distance)
{
    m_center -= m_back * distance;
    return *this;
}

Camera& Camera::Strafe(float distance)
{
    m_center += distance * m_right;
    return *this;
}

Camera& Camera::Rise(float distance)
{
    m_center += distance * m_up;
    return *this;
}

Camera& Camera::Rotate( glm::vec3 axis, float rads )
{
    Quat q( rads, axis );
    m_right = q.Rotate(m_right);
    m_up = q.Rotate(m_up);
    m_back = q.Rotate(m_back);
    return *this;
}

glm::mat4 Camera::Model() const     
{
    return glm::mat4( { m_right, 0 }, { m_up, 0 }, { m_back, 0 }, { m_center, 1 } );
}

glm::mat4 Camera::View() const      
{
    return AffineInverse( Model() );
}

glm::mat4 Camera::Perspective() const
{
    glm::mat4 perspectiveMat = { 0 };
    perspectiveMat[0][0] = 2 * m_vpDistance / m_vpWidth;
    perspectiveMat[1][1] = 2 * m_vpDistance / m_vpHeight;
    perspectiveMat[2][2] = ( m_nearDistance + m_farDistance ) / ( m_nearDistance - m_farDistance );
    perspectiveMat[3][2] = ( 2 * m_nearDistance * m_farDistance ) / ( m_nearDistance - m_farDistance );
    perspectiveMat[2][3] = -1;

    return perspectiveMat;
}
