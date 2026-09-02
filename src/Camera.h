//**************************************************************************************************
// File   : Camera.h
// Author : Cody Melvin
// Date   : 2025/07/18
// Brief  : Header file for class containing camera information and operations
//**************************************************************************************************

#include <glm/glm.hpp>

class Camera
{
    public:
        Camera();
        Camera( float fov, float aspectRatio, float near, float far, 
                glm::vec4 genUp, glm::vec4 eye, glm::vec4 lookAt );
        Camera(const Camera& other);
        void operator=(const Camera& other);
        
        void SetViewingDimensions( float fov, float aspectRatio );
        void SetPlaneDistances( float near, float far );
        void SetOrigin(glm::vec4 eye);
        void SetOrientation( glm::vec4 genUp, glm::vec4 lookAt );

        glm::vec4 Origin() const;
        glm::vec4 Up() const;
        glm::vec4 Right() const;
        glm::vec4 Back() const;
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
        glm::vec4 m_eye;
        glm::vec4 m_right, m_back, m_up;
        float m_vpDistance, m_nearDistance, m_farDistance, m_vpWidth, m_vpHeight;
};