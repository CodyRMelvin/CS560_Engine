//**************************************************************************************************
// File   : Utils.h
// Author : Cody Melvin
// Date   : 2026/10/12
// Brief  : Interface for various utility functions and objects 
//**************************************************************************************************

#pragma once
#ifndef UTILS_H
#define UTILS_H

#include "Logger.h"

#include <string>
#include <stdexcept>
#include <format>

class Error
{
    public:
        enum Type
        {
            errorInvalid = -1,
            errorNone,
            errorMinor,
            errorSevere
        };
        static std::string TypeString( Type t ) 
        {
            switch(t)
            {
                case errorInvalid:
                    return "Invalid";
                case errorNone:
                    return "None";
                case errorMinor:
                    return "Minor";
                case errorSevere:
                    return "Severe";
                default:
                    return "This shouldn't return";
            }
        }

        Error( std::string what, Type errorType ) : m_what(what), m_type(errorType) {}
        Error( const char* what, Type errorType ) : m_what(what), m_type(errorType) {}
        ~Error() = default;

        std::string_view What() { return m_what; }
        Type GetType() const { return m_type; }
        void Throw() { throw std::runtime_error( std::format( "Error: {}. What: {}", TypeString(m_type), m_what ) ); }
        #define LogError() LobErrorBase( __FILE__, __LINE__ )
        void LogErrorBase( std::string file, int line ) 
        { 
            Logger::LogLineBase( file, line, static_cast<Logger::Severity>(m_type),"Error: {}. What: {}.", TypeString(m_type), m_what ); 
        }
        
    private:
        std::string m_what;
        Type m_type;
};

#endif
