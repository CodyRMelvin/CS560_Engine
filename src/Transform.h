//**************************************************************************************************
// File   : Transform.h
// Author : Cody Melvin
// Date   : 2026/09/19
// Brief  : Interface of class encapsulating basic object transformations
//**************************************************************************************************

#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Math.h"

#include "fastgltf/types.hpp"

class Transform
{
    public:
        Transform();
        Transform(glm::vec3 scale);
        Transform( float scaleX, float scaleY, float scaleZ );
        Transform( glm::vec3 scale, glm::vec3 translation );
        Transform( float scaleX, float scaleY, float scaleZ, 
                   float translationX, float translationY, float translationZ );
        Transform( glm::vec3 scale, glm::vec3 translation, glm::vec3 rotAxis, float rotRadians );
        Transform( fastgltf::TRS trs );

        inline glm::vec3 Scale() const { return m_scale; }
        inline glm::vec3 Translation() const { return m_translation; }
        inline Quat Rotation() const { return m_rotation; }

        inline glm::vec3& Scale() { return m_scale; }
        inline glm::vec3& Translation() { return m_translation; }
        inline Quat& Rotation() { return m_rotation; }

        inline void Scale(glm::vec3 scale) 
        { 
            m_scale = scale; 
            m_isDirty = true; 
        }

        inline void Scale( float x, float y, float z ) 
        { 
            m_scale = glm::vec3( x, y, z );
            m_isDirty = true;
        }

        inline void Translation(glm::vec3 translation) 
        { 
            m_translation = translation;
            m_isDirty = true;
        }

        inline void Translation( float x, float y, float z ) 
        { 
            m_translation = glm::vec3( x, y, z );
            m_isDirty = true;
        }

        inline void Rotation( glm::vec3 axis, float radians ) 
        { 
            m_rotation = Quat( radians, glm::normalize(axis) ); 
            m_isDirty = true; 
        }
         inline void Rotation( const Quat& q )
         {
             m_rotation = q;
             m_isDirty = true;
         }

        glm::mat4 Matrix();
        inline glm::mat4 Matrix() const { return m_matrix; }

    private:
        glm::vec3 m_scale;
        glm::vec3 m_translation;
        Quat m_rotation;

        bool m_isDirty;
        glm::mat4 m_matrix;
};


#endif
