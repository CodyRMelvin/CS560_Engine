//**************************************************************************************************
// File   : Logger.h
// Author : Cody Melvin
// Date   : 2025/07/01
// Brief  : Header for system to log info to file and command line output
//**************************************************************************************************

#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <format>   // format_string, format
#include <fstream>  // ofstream
#include <utility>  // forward
#include <string>   // string, rfind, erase, size_type, begin
#include <iostream> // cout
#include <filesystem>

//Macro wrapper to inject file name and line number
#define LogLine(...) LogLineBase( __FILE__, __LINE__, __VA_ARGS__)
#define TestLog() LogLineBase( __FILE__, __LINE__, Logger::severityError, "Test" )

class Logger
{
    public:
        enum Severity
        {
            severityInvalid = -1,
            severityInfo,
            severityWarning,
            severityError    
        };

        static void Initialize(Severity filter);

        template<class... Args>
        static void LogLineBase( std::string file, int line, Severity severity, 
                                 std::format_string<Args...> fmt, Args&&... args )
        {
            //Check if severity passes filter
            if ( severity < m_s_filter )
                return;
            //Verify file is open
            if ( !m_s_logFile.is_open() )
                m_s_logFile.open( m_s_filePath, std::ios_base::app );

            //Create a variable to hold the output and preface it with the severity
            std::string outString;
            switch (severity)
            {
                case severityInfo:
                    outString += "Info: ";
                    break;
                case severityWarning:
                    outString += "Warning: ";
                    break;
                case severityError:
                    outString += "Error: ";
                    break;
                default:
                    break;
            }

            //Remove the path leading up to the file
            std::string::size_type s = file.rfind("/");
            if ( s != std::string::npos )
            {
                file.erase( file.begin(), file.begin() + s + 1 );
            }

            //Pass in format arguments to construct string
            //credit to https://stackoverflow.com/a/67745169
            outString += std::format( fmt, std::forward<Args>(args)... );
            //Format additional details for the file and line of origin
            outString += std::format( " - in {} line {}", file, line );

            //Print to the log, if in debug print to the console
            m_s_logFile << outString << "\n";
            m_s_logFile.close();
#ifdef _DEBUG
            std::cout << outString << "\n";
#endif
        }

        static void LogLineBase( std::string file, int line, Severity severity, std::string string )
        {
            //Check if severity passes filter
            if ( severity < m_s_filter )
                return;
            //Verify file is open
            if ( !m_s_logFile.is_open() )
                m_s_logFile.open( m_s_filePath, std::ios_base::app );

            //Create a variable to hold the output and preface it with the severity
            std::string outString;
            switch (severity)
            {
                case severityInfo:
                    outString += "Info: ";
                    break;
                case severityWarning:
                    outString += "Warning: ";
                    break;
                case severityError:
                    outString += "Error: ";
                    break;
                default:
                    break;
            }

            //Remove the path leading up to the file
            std::string::size_type s = file.rfind("/");
            if ( s != std::string::npos )
            {
                file.erase( file.begin(), file.begin() + s + 1 );
            }

            outString += string;
            //Format additional details for the file and line of origin
            outString += std::format( " - in {} line {}", file, line );

            //Print to the log, if in debug print to the console
            m_s_logFile << outString << "\n";
            m_s_logFile.close();
#ifdef _DEBUG
            std::cout << outString << "\n";
#endif
        }

        static void Shutdown();

    private:
        static std::filesystem::path m_s_filePath;
        static std::ofstream m_s_logFile;
        static Severity m_s_filter; 
};

#endif