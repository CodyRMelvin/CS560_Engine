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
    Application app;
    try { app.Initialize(); }
    catch( std::runtime_error e )
    {
        Logger::LogLine( Logger::severityError, "{}", e.what() );
        app.Shutdown();
        return -1;
    }
    try { app.Run(); }
    catch( std::runtime_error e )
    {
        Logger::LogLine( Logger::severityError, "{}", e.what() );
        app.Shutdown();
        return -1;
    }
    app.Shutdown();
    return 0;
}


