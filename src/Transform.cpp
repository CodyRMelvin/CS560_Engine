//**************************************************************************************************
// File   : Transform.cpp
// Author : Cody Melvin
// Date   : 2026/09/19
// Brief  : Implementation of class encapsulating basic object transforms 
//**************************************************************************************************

#include "Transform.h"

#include "glm/ext/matrix_transform.hpp"

Transform::Transform()
    : m_scale(1)
    , m_translation(0)
    , m_rotation()
    , m_isDirty(false)
    , m_matrix(0)
{}

Transform::Transform(glm::vec3 scale)
    : m_scale(scale)
    , m_translation(0)
    , m_rotation()
    , m_isDirty(true)
    , m_matrix(0)
{}

Transform::Transform( float scaleX, float scaleY, float scaleZ )
    : m_scale( scaleX, scaleY, scaleZ )
    , m_translation(0)
    , m_rotation()
    , m_isDirty(true)
    , m_matrix(0)
{}

Transform::Transform( glm::vec3 scale, glm::vec3 translation )
    : m_scale(scale)
    , m_translation(translation)
    , m_rotation()
    , m_isDirty(true)
    , m_matrix(0)
{}

Transform::Transform( float scaleX, float scaleY, float scaleZ, float translationX, float translationY, float translationZ )
    : m_scale( scaleX, scaleY, scaleZ )
    , m_translation( translationX, translationY, translationZ )
    , m_rotation()
    , m_isDirty(true)
    , m_matrix(0)
{}

Transform::Transform( glm::vec3 scale, glm::vec3 translation, glm::vec3 rotAxis, float rotRadians )
    : m_scale(scale)
    , m_translation(translation)
    , m_rotation( rotRadians, glm::normalize(rotAxis) )
    , m_isDirty(true)
    , m_matrix(0)
{}

Transform::Transform( fastgltf::TRS trs )
    : m_scale( trs.scale.x(), trs.scale.y(), trs.scale.z() )
    , m_translation( trs.translation.x(), trs.translation.y(), trs.translation.z() )
    , m_rotation( trs.rotation.x(), trs.rotation.y(),  trs.rotation.z(),  trs.rotation.w() )
    , m_isDirty(true)
    , m_matrix(0)
{ }

glm::mat4 Transform::Matrix()
{
    if (!m_isDirty)
        return m_matrix;

    m_matrix = glm::translate( glm::mat4(1), m_translation ) * 
               m_rotation.RotationMatrix() * glm::scale( glm::mat4(1), m_scale );

    return m_matrix;
}

