//**************************************************************************************************
// File   : Main.cpp
// Author : Cody Melvin
// Date   : 2025/06/13
// Brief  : Entry point of program
//**************************************************************************************************

#define FLATSHADE 1

#include "Common.h"
#include "ShaderLibrary.h"
#include "Camera.h"
#include "Window.h"
#include "Object.h"
#include "Parser.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

int main ( int argc, char* argv[] )
{
    Logger::Initialize(Logger::severityInfo);
    Logger::LogLine( Logger::severityInfo, "Logger Initialized" );

    Window window( 800, 600 ); 
    try { window.Initialize(); }
    catch (std::runtime_error e)
    {
        Logger::LogLine( Logger::severityError, e.what() );
        return -1;
    }
    
    if ( !gladLoadGL() )
    {
        Logger::LogLine( Logger::severityError, "gladLoadGL failure" );
        Logger::Shutdown();
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);

    SDL_Event e;
    bool quit = false;

    std::filesystem::path dir = std::filesystem::current_path();
    std::string pathStr = dir.string();
    std::string::size_type size = pathStr.find("/bin");
    if ( size != std::string::npos )
    {
        pathStr.erase( pathStr.begin() + size, pathStr.end() );
    }
    pathStr += "/Assets";
    glEnableVertexAttribArray(0);
    ShaderLibrary::CompileShaders( pathStr + "/shaders" );

    //Create a pipeline object, attach the shaders, and link
    GLuint pipeline;
    glGenProgramPipelines( 1, &pipeline );

#if FLATSHADE
    Shader vs = ShaderLibrary::GetShader("FlatShade", Shader::shaderTypeVertex );
    vs.Link();
    glUseProgramStages( pipeline, GL_VERTEX_SHADER_BIT, vs.Program() );
    Shader fs = ShaderLibrary::GetShader("FlatShade", Shader::shaderTypeFragment);
    fs.Link();
    glUseProgramStages( pipeline, GL_FRAGMENT_SHADER_BIT, fs.Program() );
#else
    Shader vs = ShaderLibrary::GetShader("demo", Shader::shaderTypeVertex );
    vs.Link();
    glUseProgramStages( pipeline, GL_VERTEX_SHADER_BIT, vs.Program() );
    Shader fs = ShaderLibrary::GetShader("demo", Shader::shaderTypeFragment);
    fs.Link();
    glUseProgramStages( pipeline, GL_FRAGMENT_SHADER_BIT, fs.Program() );
#endif

    //Tell opengl this pipeline is currently being used
    glBindProgramPipeline(pipeline);

    dir = std::filesystem::path( pathStr + "/glTF/roman_D_out/roman_D.gltf" );   
    Parser parser( dir, fastgltf::Options::LoadExternalBuffers );
    if (!parser)
    {
        Logger::LogLine( Logger::severityError, "Failed to construct parser" );
        return -1;
    }

    std::vector<Mesh> meshes;
    if ( !parser.ReadMeshes(meshes) )
    {
        Logger::LogLine( Logger::severityError, "Failed to parse meshes" );
        return -1;
    }

    Object obj( meshes[0], Transform( 1, 1, 1 ) );
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Camera cam = Camera().SetViewingDimensions( 90.f, 4.f / 3.f )
                         .SetPlaneDistances( .1f, 100.f )
                         .LookAt( { 0, 0, 10 }, { 0, 0, 0 }, { 0, 1, 0 } );

    glUseProgram( vs.Program() );
#if FLATSHADE
    glm::mat4 normalMatrix = cam.Model();
    normalMatrix[3][0] = normalMatrix[3][1] = 0;
    normalMatrix = AffineInverse(normalMatrix);
    normalMatrix = glm::transpose(normalMatrix);

    glUniformMatrix4fv( 3, 1, GL_FALSE, glm::value_ptr( normalMatrix ) );
#endif

    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( cam.View() ) );
    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( cam.Perspective() ) );
    glUseProgram(0);

    Logger::LogLine( Logger::severityInfo, "Beginning program loop" );
    while (!quit)
    {
        glClearDepth(1);
        glClearColor( 0, 0, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        //for ( Object& obj : objects )
            //obj.Render(vs.Program());
        obj.Render( vs.Program() );
        //obj2.Render( vs.Program() );

        glFlush();

        window.SwapWindow();

        while ( SDL_PollEvent(&e) )
        {
            switch(e.type)
            {
                case SDL_EVENT_QUIT:
                    Logger::LogLine( Logger::severityInfo, "Event quit" );
                    quit = true;
                    break;
                
                case SDL_EVENT_KEY_DOWN:
                    if (e.key.key == SDLK_W)
                        cam.Forwards(.5);
                    if (e.key.key == SDLK_S)
                        cam.Forwards(-.5);
                    if (e.key.key == SDLK_D)
                        cam.Strafe(.5);
                    if (e.key.key == SDLK_A)
                        cam.Strafe(-.5);
                    
                    if (e.key.key == SDLK_UP)
                        cam.Rotate( { 1, 0, 0 }, .05 );
                    if (e.key.key == SDLK_DOWN)
                        cam.Rotate( { 1, 0, 0 }, -.05 );
                    if (e.key.key == SDLK_LEFT)
                        cam.Rotate( { 0, 1, 0 }, .05 );
                    if (e.key.key == SDLK_RIGHT)
                        cam.Rotate( { 0, 1, 0 }, -.05 );

                    if ( e.key.key == SDLK_Q )
                    {
                        SDL_Event eQuit;
                        SDL_zero(eQuit);
                        eQuit.type = SDL_EVENT_QUIT;
                        SDL_PushEvent(&eQuit);
                    }

                    glUseProgram( vs.Program() );
                    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( cam.View() ) );
                    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( cam.Perspective() ) );
                    glUseProgram(0);
                    break;
            }
        }
    }
    
    //Delete the program, vertex buffer, and vao
    glDeleteProgramPipelines( 1, &pipeline );
    ShaderLibrary::DeleteShaders();
    //for ( Object& obj : objects )
        obj.Destroy();

    Logger::Shutdown();

    return 0;
}


