//**************************************************************************************************
// File   : Camera.h
// Author : Cody Melvin
// Date   : 2025/07/18
// Brief  : Implementation file for class containing camera information and operations
//**************************************************************************************************

#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cmath>

Camera::Camera()
    : m_eye( { 0, 0, 0, 1 } )
    , m_right( { 1, 0, 0, 0 } )
    , m_back( { 0, 0, 1, 0 } )
    , m_up( { 0, 1, 0, 0 } )
    , m_vpDistance(5.05)
    , m_nearDistance(.1)
    , m_farDistance(10)
    , m_vpWidth(10.1)
    , m_vpHeight(10.1)
{ }

Camera::Camera( float fov, float aspectRatio, float near, float far,
                glm::vec4 genUp, glm::vec4 eye, glm::vec4 lookAt )
    : m_eye(eye)
    , m_right( glm::vec4( glm::normalize( glm::cross( glm::vec3(lookAt), glm::vec3(genUp) ) ), 0 ) )
    , m_back( -( glm::normalize(lookAt) ) )
    , m_up( glm::vec4( glm::cross( glm::vec3(m_back), glm::vec3(m_right) ), 0 ) )
    , m_vpDistance( ( far - near ) / 2 + near )
    , m_nearDistance(near)
    , m_farDistance(far) 
    , m_vpWidth( 2 * m_vpDistance * std::tan( glm::radians(fov) * .5 ) )
    , m_vpHeight( m_vpWidth / aspectRatio )
{ }

Camera::Camera(const Camera& other)
    : m_eye(other.m_eye)
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

void Camera::SetViewingDimensions( float fov, float aspectRatio )
{
    m_vpWidth = 2 * m_vpDistance * std::tan( glm::radians(fov) * .5 );
    m_vpHeight = m_vpWidth / aspectRatio;
}

void Camera::SetPlaneDistances( float near, float far )
{
    float oldDistance = m_vpDistance;
    m_nearDistance = near;
    m_farDistance = far;
    m_vpDistance = ( far - near ) / 2 + near;

    float aspectRatio = m_vpWidth / m_vpHeight;
    float fov = std::atan( m_vpWidth / (2 * oldDistance) ) / 2;

    SetViewingDimensions( fov, aspectRatio );
}

void Camera::SetOrigin(glm::vec4 eye)
{
    Forwards( (m_eye - eye).length() );
}

void Camera::SetOrientation( glm::vec4 genUp, glm::vec4 lookAt )
{
    m_right = glm::vec4( glm::normalize( glm::cross( glm::vec3(lookAt), glm::vec3(genUp) ) ), 0 );
    m_back = -( glm::normalize(lookAt) );
    m_up = glm::vec4( glm::cross( glm::vec3(m_right), glm::vec3(m_back) ), 0 );
}

glm::vec4 Camera::Origin() const
{
    return m_eye;
}

glm::vec4 Camera::Up() const
{
    return m_up;
}

glm::vec4 Camera::Right() const
{
    return m_right;
}

glm::vec4 Camera::Back() const
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
    m_eye -= m_back * distance;

    // float ratio = m_vpWidth / m_vpHeight;
    // m_vpWidth /= m_vpDistance;
    // m_vpWidth *= distance;
    // m_vpHeight = m_vpWidth / ratio;

    // m_vpDistance += distance;
    // m_nearDistance += distance;
    // m_farDistance += distance;

    return *this;
}

Camera& Camera::Strafe(float distance)
{
    m_eye += distance * m_right;
    return *this;
}

Camera& Camera::Rise(float distance)
{
    m_eye += distance * m_up;
    return *this;
}

Camera& Camera::Rotate( glm::vec3 axis, float rads )
{
    glm::mat4 rot(1);
    rot = glm::rotate( rot, rads, axis );
    m_up = rot * m_up;
    m_right = rot * m_right;
    m_back = rot * m_back;

    return *this;
}

glm::mat4 Camera::Model() const     
{
    return glm::mat4( m_right, m_up, m_back, m_eye );
}

glm::mat4 Camera::View() const      
{
    glm::mat3 inverse( m_right, m_up, m_back );
    inverse = glm::inverse(inverse);

    glm::mat4 result(inverse);
    result[3][3] = 1;

    glm::vec3 invEye = -m_eye;
    result = glm::translate( result, invEye );

    return result;
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
