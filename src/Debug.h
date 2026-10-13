//**************************************************************************************************
// File   : Debug.h
// Author : Cody Melvin
// Date   : 2026/10/13
// Brief  : Simple utility functions and objects for debugging 
//**************************************************************************************************

#ifdef _DEBUG
#pragma once
#ifndef STUB_H
#define STUB_H

#include "Common.h"

#include "glad/glad.h"

void glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    Logger::Severity sev = Logger::severityInvalid;

    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            sev = Logger::severityError;
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            sev = Logger::severityError;
            break;

        case GL_DEBUG_SEVERITY_LOW:
            sev = Logger::severityWarning;
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            sev = Logger::severityInfo;
            break;

        default:
            break;
    }

    std::string sourceStr;

    switch(source)
    {
        case GL_DEBUG_SOURCE_API:
            sourceStr = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceStr = "Window system";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceStr = "Shader compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceStr = "Third party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceStr = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceStr = "Other";
            break;

        default:
            break;
    }

    std::string typeStr;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               
            typeStr = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: 
            typeStr = "Deprecated behavior";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:   
            typeStr = "Undefined behavior";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:         
            typeStr = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:         
            typeStr = "Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:              
            typeStr = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          
            typeStr = "Push group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:           
            typeStr = "Pop group";
            break;
        case GL_DEBUG_TYPE_OTHER:               
            typeStr = "Other";
            break;

        default:
            break;
    }

    Logger::LogLine( sev, "Source: {}, type: {}, id: {}, length: {}, message: {}, userParam: {}",
                     sourceStr, typeStr, id, length, message, userParam );
}

#endif
#endif
