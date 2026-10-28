//**************************************************************************************************
// File   : Camera.h
// Author : Cody Melvin
// Date   : 2025/07/18
// Brief  : Implementation file for class containing camera information and operations
//**************************************************************************************************

#include "Camera.h"

#include "Common.h"
#include "glm/ext/matrix_clip_space.hpp"

#include <glm/ext/matrix_transform.hpp>

Camera::Camera()
    : Node3D()
    , m_center( { 0, 0, 0 } )
    , m_right( { 1, 0, 0 } )
    , m_back( { 0, 0, 1 } )
    , m_up( { 0, 1, 0 } )
    , m_zNear(.1)
    , m_zFar(1000)
    , m_fov( glm::radians(90.f) )
    , m_aspect( 4.f / 3.f )
{ }

Camera::Camera( uint64_t id, uint64_t parent )
    : Node3D( id, parent )
    , m_center( { 0, 0, 0 } )
    , m_right( { 1, 0, 0 } )
    , m_back( { 0, 0, 1 } )
    , m_up( { 0, 1, 0 } )
    , m_zNear(.1)
    , m_zFar(1000)
    , m_fov( glm::radians(90.f) )
    , m_aspect( 4.f / 3.f )
{ }

Camera::Camera(const Camera& other)
    : Node3D(other)
    , m_center(other.m_center)
    , m_right(other.m_right)
    , m_back(other.m_back)
    , m_up(other.m_up)
    , m_zNear(other.m_zNear)
    , m_zFar(other.m_zFar)
    , m_fov(other.m_fov)
    , m_aspect(other.m_aspect)
{ }

void Camera::operator=(const Camera& other)
{
    *this = Camera(other);
}

void Camera::Initialize() 
{
    /*
    Transform t = GlobalTransform();
    glm::vec3 up = t.Rotation().Rotate( { 0, 1, 0 } );
    glm::vec3 lookat = t.Rotation().Rotate( { 0, 0, -1 } );
    LookAt( t.Translation(), lookat, up );
    */
}

Camera& Camera::SetViewingDimensions( float degFov, float aspectRatio )
{
    m_fov = glm::radians(degFov);
    m_aspect = aspectRatio;
    return *this;
}

Camera& Camera::SetPlaneDistances( float near, float far )
{
    m_zNear = near;
    m_zFar = far;

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

/*glm::vec3 Camera::Viewport() const
{
    return glm::vec3( m_vpWidth, m_vpHeight, m_vpDistance );
}*/

float Camera::Near() const
{
    return m_zNear;
}

float Camera::Far() const
{
    return m_zFar;
}

Camera& Camera::Zoom(float factor)
{
    m_fov /= factor;
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

Camera& Camera::Rotate( glm::vec3 localAxis, float rads )
{
    glm::vec4 localAxis4 = glm::vec4( localAxis, 0 ) * View();
    Quat q( rads, localAxis4 );
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
    return glm::perspective( m_fov, m_aspect, m_zNear, m_zFar );
}
