//**************************************************************************************************
// File   : Logger.hpp
// Author : Cody Melvin
// Date   : 2025/07/01
// Brief  : Header for system to log info to file and command line output
//**************************************************************************************************

#include <format>   // format_string, format
#include <fstream>  // ofstream
#include <utility>  // forward
#include <string>   // string, rfind, erase, size_type, begin
#include <iostream> // cout

#ifdef FALSE
//Ugly macro bullshit to allow for logline without any additional args
//Credit to https://stackoverflow.com/a/11763277
#define EXPAND(x)                           x
#define GET_MACRO(_1, _2, _3, name, ...)    name
#define LogLine(...)    EXPAND( GET_MACRO( __VA_ARGS__, LogLine3, LogLine2 ) (__VA_ARGS__) )

//Macro wrappers to inject file name and line number
#define LogLine3( severity, fmt, args ) LogLineBase( severity, __FILE__, __LINE__, fmt, args )
#define LogLine2( severity, fmt ) LogLineBase( severity, __FILE__, __LINE__, fmt )
#endif

//Macro wrapper to inject file name and line number
#define LogLine(...) LogLineBase( __FILE__, __LINE__, __VA_ARGS__)

class Logger
{
    public:
        enum Severity
        {
            severityInfo,
            severityWarning,
            severityError    
        };

        static void Initialize(Severity filter);

        template<class... Args>
        static void LogLineBase( std::string file, int line, Severity severity, 
                                 std::format_string<Args...> fmt, Args&&... args )
        {
            //Verify file is open
            if ( !m_s_logFile.is_open() )
                return;
            //Check if severity passes filter
            if ( severity < m_s_filter )
                return;

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
#ifdef _DEBUG
            std::cout << outString << "\n";
#endif
        }

        static void Shutdown();

    private:
        static std::ofstream m_s_logFile;
        static Severity m_s_filter; 
};