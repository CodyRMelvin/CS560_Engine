//**************************************************************************************************
// File   : Camera.h
// Author : Cody Melvin
// Date   : 2025/07/18
// Brief  : Header file for class containing camera information and operations
//**************************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include "Common.h"

class Camera
{
    public:
        Camera();
        Camera(const Camera& other);
        void operator=(const Camera& other);
        
        Camera& SetViewingDimensions( float fov, float aspectRatio );
        Camera& SetPlaneDistances( float near, float far );
        Camera& LookAt( glm::vec3 position, glm::vec3 lookAt, glm::vec3 up );

        glm::vec3 Origin() const;
        glm::vec3 Up() const;
        glm::vec3 Right() const;
        glm::vec3 Back() const;
        glm::vec3 Viewport() const;
        float Near() const;
        float Far() const;

        Camera& Zoom(float factor);
        Camera& Forwards(float distance);
        Camera& Strafe(float distance);
        Camera& Rise(float distance);
        Camera& Rotate( glm::vec3 axis, float rads );

        glm::mat4 Model() const;
        glm::mat4 View() const;
        glm::mat4 Perspective() const;

    private:
        glm::vec3 m_center;
        glm::vec3 m_right, m_back, m_up;
        float m_vpDistance, m_nearDistance, m_farDistance, m_vpWidth, m_vpHeight;
};

#endif