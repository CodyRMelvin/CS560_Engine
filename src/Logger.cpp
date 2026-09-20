//**************************************************************************************************
// File   : Logger.hpp
// Author : Cody Melvin
// Date   : 2025/13/01
// Brief  : Implementation for system to log info to file and command line output
//**************************************************************************************************

#include "Logger.h"

#include <fstream>    // ofstream, open, is_open, close
#include <chrono>     // now, year_month_day, floor, days, hh_mm_ss, seconds, current_zone, 
                      // to_local, time_point_cast, ratio, duration_cast, time_point
#include <string>     // string
#include <format>     // format
#include <filesystem> // create_directory, path

std::filesystem::path Logger::m_s_filePath;
std::ofstream Logger::m_s_logFile;
Logger::Severity Logger::m_s_filter = Logger::severityInfo;

void Logger::Initialize(Severity filter)
{
    if ( m_s_logFile.is_open() )
        return;

    // Get the current time from chrono and convert that into year/month/day
    const std::chrono::time_point c_now{ std::chrono::system_clock::now() };
    const std::chrono::year_month_day c_date{ std::chrono::floor<std::chrono::days>(c_now) };

    // Convert c_now into the timezone used by the machine
    const std::chrono::time_point c_timeZoned = std::chrono::current_zone()->to_local(c_now);
    auto tod = c_timeZoned - std::chrono::
        time_point_cast< std::chrono::duration< int, std::ratio<86400> > >(c_timeZoned);

    // Get individual hours, minutes, seconds
    // Credit goes to https://stackoverflow.com/a/77442349
    auto hour = std::chrono::duration_cast<std::chrono::hours>(tod);
    tod -= hour;
    auto minute = std::chrono::duration_cast<std::chrono::minutes>(tod);
    tod -= minute;
    auto second = std::chrono::duration_cast<std::chrono::seconds>(tod);
    
    //Format that into a text file in the Logs foler labeled "yyyy-mm-dd_hh:mm:ss_long.txt"
    const std::string c_fileName = std::format
    ( 
        "Logs/{}_{}:{}:{}_log.txt"
        , c_date
        , hour.count()
        , minute.count()
        , second.count()
    ); 
    
    //Create the Logs folder if it doesn't exist
    std::filesystem::create_directory("./Logs");
    //Create the paht to the log file
    m_s_filePath = std::filesystem::path(c_fileName);

    m_s_filter = filter;

}

void Logger::Shutdown()
{
    if ( !m_s_logFile.is_open() )
        return;

    m_s_logFile.close();
    m_s_filePath.clear();
}
