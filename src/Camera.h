//**************************************************************************************************
// File   : Camera.h
// Author : Cody Melvin
// Date   : 2025/07/18
// Brief  : Header file for class containing camera information and operations
//**************************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include "Common.h"
#include "Node.h"

class Camera : public Node3D
{
    public:
        Camera();
        Camera( uint64_t id, uint64_t parent );
        Camera(const Camera& other);
        void operator=(const Camera& other);

        void Initialize() override;
        void Render() override {}
        void Destroy() override {}
        
        Camera& SetViewingDimensions( float fov, float aspectRatio );
        Camera& SetPlaneDistances( float near, float far );
        Camera& LookAt( glm::vec3 position, glm::vec3 lookAt, glm::vec3 up );

        glm::vec3 Origin() const;
        glm::vec3 Up() const;
        glm::vec3 Right() const;
        glm::vec3 Back() const;
        //glm::vec3 Viewport() const;
        float Near() const;
        float Far() const;

        Camera& Zoom(float factor);
        Camera& Forwards(float distance);
        Camera& Strafe(float distance);
        Camera& Rise(float distance);
        Camera& Rotate( glm::vec3 localAxis, float rads );

        glm::mat4 Model() const;
        glm::mat4 View() const;
        glm::mat4 Perspective() const;

    private:
        glm::vec3 m_center;
        glm::vec3 m_right, m_back, m_up;
        float m_zNear, m_zFar;
        float m_fov, m_aspect;
};

#endif