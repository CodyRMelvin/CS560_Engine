//**************************************************************************************************
// File   : Main.cpp
// Author : Cody Melvin
// Date   : 2025/06/13
// Brief  : Entry point of program
//**************************************************************************************************

#include "fastgltf/types.hpp"
#define WIRE 0
#define FLATSHADE 0

#include "Logger.h"
#include "ShaderLibrary.h"
#include "Camera.h"
#include "Math.h"
#include "Window.h"
#include "Object.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"

#include <vector>
#include <filesystem>

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

    fastgltf::Parser parser;
    dir = std::filesystem::path( pathStr + "/glTF/roman_D_out/roman_D.gltf" );

    auto gltfStream = fastgltf::GltfDataBuffer::FromPath(dir);
    if ( gltfStream.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityError, "Failed to read gltf. Error number {}", static_cast<uint64_t>( gltfStream.error() ) );
        return -1;
    }


    auto asset = parser.loadGltf( gltfStream.get(), dir.parent_path(), fastgltf::Options::LoadExternalBuffers );
    if ( asset.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityError, "Failed to parse gltf. Error number {}", static_cast<uint64_t>( gltfStream.error() ) );
        return -1;
    }

    std::vector<Object> objects;

    for ( const auto& mesh : asset->meshes )
    {
        for ( auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it )
        {
            const auto* posIt =  it->findAttribute("POSITION"); 
            assert( posIt != it->attributes.end() );
            assert( it->indicesAccessor.has_value() );

            size_t baseColorTexcoordIndex = 0;
            auto index = std::distance( mesh.primitives.begin(), it );
            if ( it->materialIndex.has_value() )
            {
                auto& material = asset->materials[ it->materialIndex.value() ];
                auto& baseColorTexture = material.pbrData.baseColorTexture;
                if ( baseColorTexture.has_value() )
                {
                    auto& texture = asset->textures[baseColorTexture->textureIndex];
                    if ( !texture.imageIndex.has_value() )
                    {
                        Logger::LogLine( Logger::severityError, "Mesh {} in file {} has no image index", 
                                         std::distance( mesh.primitives.begin(), it ), dir.parent_path().string() );
                        return -1;
                    }

                    if ( baseColorTexture->transform && baseColorTexture->transform->texCoordIndex.has_value() )
                        baseColorTexcoordIndex = baseColorTexture->transform->texCoordIndex.value();
                    else
                        baseColorTexcoordIndex = material.pbrData.baseColorTexture->texCoordIndex;
                }
            }

            auto& posAccessor = asset->accessors[posIt->accessorIndex]; 
            if ( !posAccessor.bufferViewIndex.has_value() )
                continue;

            std::vector<Vertex> vertices;
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( asset.get(), posAccessor, 
            [&vertices]( fastgltf::math::fvec3 pos, std::size_t idx )
            {
                Vertex v;
                v.pos = glm::vec3( pos.x(), pos.y(), pos.z() );
                v.nrm = glm::vec3();
                v.tex = glm::vec2();
                vertices.push_back(v);
            });

            const auto* texcoordAttribute = it->findAttribute( std::string("TEXCOORD_") + std::to_string(baseColorTexcoordIndex) );  
            if ( texcoordAttribute != it->attributes.end() ) 
            {
                auto& texCoordAccessor = asset->accessors[texcoordAttribute->accessorIndex];
                if ( !texCoordAccessor.bufferViewIndex.has_value() )
                    continue;

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>( asset.get(), texCoordAccessor,
                [&vertices]( fastgltf::math::fvec2 uv, size_t idx )
                {
                    if( vertices.capacity() < idx )
                        vertices.resize( vertices.capacity() * 3 / 2 );

                    vertices[idx].tex = glm::vec2( uv.x(), uv.y() );
                }); 
            }

            const auto* normalAttribute = it->findAttribute("NORMAL"); 
            if ( normalAttribute != it->attributes.end() )
            {
                auto& normalAccessor = asset->accessors[normalAttribute->accessorIndex];
                if ( !normalAccessor.bufferViewIndex.has_value() )
                    continue;

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( asset.get(), normalAccessor,
                [&vertices]( fastgltf::math::fvec3 nrm, size_t idx )
                {
                    if( vertices.capacity() < idx )
                        vertices.resize( vertices.capacity() * 3 / 2 );

                    vertices[idx].nrm = glm::vec3( nrm.x(), nrm.y(), nrm.z() );
                }); 

            }

            auto& indexAccessor = asset->accessors[ it->indicesAccessor.value() ];
            if ( !indexAccessor.bufferViewIndex.has_value() )
            {
                Logger::LogLine( Logger::severityError, "Mesh {} in file {} has no indices buffer view", 
                                 std::distance( mesh.primitives.begin(), it ), dir.parent_path().string() );
                return -1;
            }

            std::vector<uint32_t> indices;
            indices.reserve(indexAccessor.count);

            fastgltf::iterateAccessorWithIndex<uint32_t>( asset.get(), indexAccessor, 
            [&indices]( uint32_t index, size_t idx )
            {
                indices.push_back(index);
            });

            objects.emplace_back( Mesh( vertices, indices ), Transform( 1, 1, 1, 0, -1, 2 ) );
        }
    }

    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    Camera cam( 70.f, 4.f / 3.f, .1f, 10.f, glm::vec4( 0, 1, 0, 0 ), 
                glm::vec4( 0, 0, 0, 1 ), glm::vec4( 0, 0, 1, 0 ) );

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

    while (!quit)
    {
        glClearDepth(1);
        glClearColor( 0, 0, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        //for ( Object& obj : objects )
            //obj.Render(vs.Program());
        objects[0].Render( vs.Program() );

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
                        cam.Forwards(.05);
                    else if (e.key.key == SDLK_S)
                        cam.Forwards(-.05);
                    else if (e.key.key == SDLK_D)
                        cam.Strafe(.05);
                    else if (e.key.key == SDLK_A)
                        cam.Strafe(-.05);
                    
                    if (e.key.key == SDLK_UP)
                        cam.Rotate( { 1, 0, 0 }, -.02 );
                    else if (e.key.key == SDLK_DOWN)
                        cam.Rotate( { 1, 0, 0 }, .02 );
                    else if (e.key.key == SDLK_LEFT)
                        cam.Rotate( { 0, 1, 0 }, -.02 );
                    else if (e.key.key == SDLK_RIGHT)
                        cam.Rotate( { 0, 1, 0 }, .02 );

                    if ( e.key.key == SDLK_Q )
                    {
                        SDL_Event eQuit;
                        SDL_zero(eQuit);
                        eQuit.type = SDL_EVENT_QUIT;
                        SDL_PushEvent(&eQuit);
                    }

                    

                    glUseProgram( vs.Program() );
                    glUniformMatrix4fv( 1, 1, GL_FALSE, glm::value_ptr( cam.Model() ) );
                    glUniformMatrix4fv( 2, 1, GL_FALSE, glm::value_ptr( cam.Perspective() ) );
                    glUseProgram(0);
                    break;
            }
        }
    }
    
    //Delete the program, vertex buffer, and vao
    glDeleteProgramPipelines( 1, &pipeline );
    ShaderLibrary::DeleteShaders();
    for ( Object& obj : objects )
        obj.Destroy();
    Logger::Shutdown();

    return 0;
}


