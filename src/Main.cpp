//**************************************************************************************************
// File   : Main.cpp
// Author : Cody Melvin
// Date   : 2025/06/13
// Brief  : Entry point of program
//**************************************************************************************************

#include "Application.h"
#include <stdexcept>

int main ( int argc, char* argv[] )
{
    // Create application
    Application& app = Application::Get();
    // Initialize appliaction and catch any runtime errors
    try { app.Initialize(); }
    catch( std::runtime_error e )
    {
        Logger::LogLine( Logger::severityError, "{}", e.what() );
        app.Shutdown();
        return -1;
    }
    // Run application and catch any runtime errors
    try { app.Run(); }
    catch( std::runtime_error e )
    {
        Logger::LogLine( Logger::severityError, "{}", e.what() );
        app.Shutdown();
        return -1;
    }
    // Shutdown application when runtime loop exits
    app.Shutdown();
    return 0;
}


