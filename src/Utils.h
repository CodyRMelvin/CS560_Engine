//**************************************************************************************************
// File   : Utils.h
// Author : Cody Melvin
// Date   : 2026/10/12
// Brief  : Interface for various utility functions and objects 
//**************************************************************************************************

#pragma once
#include <filesystem>
#ifndef UTILS_H
#define UTILS_H

#include "Logger.h"

#include <string>
#include <stdexcept>
#include <format>

// Class to represent errors, primarily for std expecteds
class Error
{
    public:
        // Enum stated type of error
        enum Type
        {
            errorInvalid = -1,
            errorNone,
            errorMinor,
            errorSevere
        };

        // Static method to convert type to a string
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

        // Constuctors and destructor
        Error( std::string what, Type errorType ) : m_what(what), m_type(errorType) {}
        Error( const char* what, Type errorType ) : m_what(what), m_type(errorType) {}
        ~Error() = default;

        // Getters
        std::string_view What() { return m_what; }
        Type GetType() const { return m_type; }
        // Method to throw runtime error with this error's information
        void Throw() { throw std::runtime_error( std::format( "Error: {}. What: {}", TypeString(m_type), m_what ) ); }
        // Wrappers around LogLine to log this error and preserve file and line location
        #define LogError() LogErrorBase( __FILE__, __LINE__ )
        void LogErrorBase( std::string file, int line ) 
        { 
            Logger::LogLineBase( file, line, static_cast<Logger::Severity>(m_type),"Error: {}. What: {}.", TypeString(m_type), m_what ); 
        }
        
    private:
        std::string m_what;
        Type m_type;
};

// Static getter for current working directory
inline std::filesystem::path GetCWD()
{
    static std::filesystem::path dir = std::filesystem::current_path();
    return dir; 
}

#endif
