//**************************************************************************************************
// File   : Math.h
// Author : Cody Melvin
// Date   : 2026/09/01
// Brief  : Public interface of variety of math utilities
//*************************************************************************************************

#ifndef MATH_H
#define MATH_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fastgltf/math.hpp>

#include "Quaternion.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 nrm;
    glm::vec2 tex;
};

inline glm::mat4 AffineInverse(glm::mat4 M)
{
    glm::mat3 inverse( M );
    inverse = glm::inverse(inverse);

    glm::mat4 result(inverse);
    result[3][3] = 1;

    glm::vec3 invTranslate = -M[3];
    result = glm::translate( result, invTranslate );

    return result;
}

#endif
