//**************************************************************************************************
// File   : Parser.cpp
// Author : Cody Melvin
// Date   : 2026/10/09
// Brief  : Implementation of class used to parse files
//**************************************************************************************************

#include "Parser.h"
#include "Mesh.h"

#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"

fastgltf::Parser Parser::m_parser;

Parser::Parser( std::string path, fastgltf::Options options  )
    : m_filePath(path) 
    , m_asset() 
    , m_buffer()
    , m_opened(false)
{
    auto expectedBuf = fastgltf::GltfDataBuffer::FromPath(m_filePath);
    if ( expectedBuf.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityWarning, "Failed to read gltf buffer from path {}, error: {}", 
                         path, fastgltf::getErrorMessage( expectedBuf.error() ) );
        return;
    }

    m_buffer = std::move( expectedBuf.get() );
    auto expectedAsset = m_parser.loadGltf( m_buffer, m_filePath.parent_path(), options );
    if ( expectedAsset.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityWarning, "Failed to load gltf asset from path {}, error: {}", 
                         path, fastgltf::getErrorMessage( expectedAsset.error() ) );
        return;
    }

    m_asset = std::move( expectedAsset.get() );
    m_opened = true;
}

Parser::Parser( std::filesystem::path path, fastgltf::Options options  )
    : m_filePath(path) 
    , m_asset() 
    , m_buffer()
    , m_opened(false)
{
    auto expectedBuf = fastgltf::GltfDataBuffer::FromPath(m_filePath);
    if ( expectedBuf.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityWarning, "Failed to read gltf buffer from path {}, error: {}", 
                         path.string(), fastgltf::getErrorMessage( expectedBuf.error() ) );
        return;
    }

    m_buffer = std::move( expectedBuf.get() );
    auto expectedAsset = m_parser.loadGltf( m_buffer, m_filePath.parent_path(), options );
    if ( expectedAsset.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityWarning, "Failed to load gltf asset from path {}, error: {}", 
                         path.string(), fastgltf::getErrorMessage( expectedAsset.error() ) );
        return;
    }

    m_asset = std::move( expectedAsset.get() );
    m_opened = true;
}

[[nodiscard]] std::expected< Mesh, Error > Parser::ReadMesh( fastgltf::Mesh& mesh ) const
{
    return std::unexpected( Error("Unknown error") );
}

[[nodiscard]] bool Parser::ReadMeshes( std::vector<Mesh>& meshes )
{
    for ( const auto& mesh : m_asset.meshes )
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for ( auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it )
        {
            const auto* posIt =  it->findAttribute("POSITION"); 
            assert( posIt != it->attributes.end() ); //Replace these with logger errors and returing false
            assert( it->indicesAccessor.has_value() );

            size_t baseColorTexcoordIndex = 0;
            auto index = std::distance( mesh.primitives.begin(), it );
            if ( it->materialIndex.has_value() )
            {
                auto& material = m_asset.materials[ it->materialIndex.value() ];
                auto& baseColorTexture = material.pbrData.baseColorTexture;
                if ( baseColorTexture.has_value() )
                {
                    auto& texture = m_asset.textures[baseColorTexture->textureIndex];
                    if ( !texture.imageIndex.has_value() )
                    {
                        Logger::LogLine( Logger::severityWarning, "Mesh {} in file {} has no image index", 
                                         std::distance( mesh.primitives.begin(), it ), m_filePath.parent_path().string() );
                        return false;
                    }

                    if ( baseColorTexture->transform && baseColorTexture->transform->texCoordIndex.has_value() )
                        baseColorTexcoordIndex = baseColorTexture->transform->texCoordIndex.value();
                    else
                        baseColorTexcoordIndex = material.pbrData.baseColorTexture->texCoordIndex;
                }
            }

            auto& posAccessor = m_asset.accessors[posIt->accessorIndex]; 
            if ( !posAccessor.bufferViewIndex.has_value() )
                continue;

            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, posAccessor, 
            [&]( fastgltf::math::fvec3 pos, std::size_t idx )
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
                auto& texCoordAccessor = m_asset.accessors[texcoordAttribute->accessorIndex];
                if ( !texCoordAccessor.bufferViewIndex.has_value() )
                    continue;

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>( m_asset, texCoordAccessor,
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
                auto& normalAccessor = m_asset.accessors[normalAttribute->accessorIndex];
                if ( !normalAccessor.bufferViewIndex.has_value() )
                    continue;

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, normalAccessor,
                [&vertices]( fastgltf::math::fvec3 nrm, size_t idx )
                {
                    if( vertices.capacity() < idx )
                        vertices.resize( vertices.capacity() * 3 / 2 );

                    vertices[idx].nrm = glm::vec3( nrm.x(), nrm.y(), nrm.z() );
                }); 

            }

            auto& indexAccessor = m_asset.accessors[ it->indicesAccessor.value() ];
            if ( !indexAccessor.bufferViewIndex.has_value() )
            {
                Logger::LogLine( Logger::severityError, "Mesh {} in file {} has no indices buffer view", 
                                 std::distance( mesh.primitives.begin(), it ), m_filePath.parent_path().string() );
                return false;
            }

            if ( indexAccessor.componentType == fastgltf::ComponentType::UnsignedByte 
                 || indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort ) 
            {
                indices.reserve(indexAccessor.count);
                fastgltf::iterateAccessorWithIndex<uint16_t>( m_asset, indexAccessor, 
                [&indices]( uint16_t index, size_t idx )
                {
                    indices.push_back( static_cast<uint32_t>(index) );
                });
            }
            else
            {
                indices.reserve(indexAccessor.count);
                fastgltf::iterateAccessorWithIndex<uint32_t>( m_asset, indexAccessor, 
                [&indices]( uint32_t index, size_t idx )
                {
                    indices.push_back(index);
                });
            }

        }

        meshes.emplace_back( vertices, indices );
        vertices.clear();
        indices.clear();
    }

    return true;
}
