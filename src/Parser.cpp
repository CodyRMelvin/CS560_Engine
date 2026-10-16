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
#include "fastgltf/tools.hpp"
#include <expected>

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

[[nodiscard]] std::expected< Mesh, Error > Parser::ReadMesh( const fastgltf::Mesh& mesh ) const
{
    Mesh m( mesh.primitives.size() );
    for ( auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it )
    {
        const fastgltf::Attribute* posIt = it->findAttribute("POSITION");

        if ( posIt == it->attributes.end() )
            return std::unexpected( Error( "No position attribute found in mesh", Error::errorMinor ) );
        if ( !it->indicesAccessor.has_value() )
            return std::unexpected( Error( "No indices accessor found in mesh", Error::errorMinor ) );

        uint16_t index = std::distance( mesh.primitives.begin(), it );

        size_t baseColorTexcoordIndex = 0;
        if ( it->materialIndex.has_value() )
        {
            const fastgltf::Material& material = m_asset.materials[ it->materialIndex.value() ];

            auto& baseColorTexture = material.pbrData.baseColorTexture;
            if ( baseColorTexture.has_value() )
            {
                const fastgltf::Texture& texture = m_asset.textures[baseColorTexture->textureIndex];
                if ( !texture.imageIndex.has_value() )
                    return std::unexpected( Error( "Mesh has no image index", Error::errorMinor ) );

                if ( baseColorTexture->transform && baseColorTexture->transform->texCoordIndex.has_value() )

                    baseColorTexcoordIndex = baseColorTexture->transform->texCoordIndex.value();
                else
                    baseColorTexcoordIndex = material.pbrData.baseColorTexture->texCoordIndex;
            }
        }

        m.InitializeVertexArray( index, fastgltf::to_underlying(it->type) ); 
        const fastgltf::Accessor& posAccessor = m_asset.accessors[posIt->accessorIndex];
        if ( !posAccessor.bufferViewIndex.has_value() )
            continue;

        m.InitializeVertexBuffer( index, posAccessor.count ); 
        Vertex* vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) );
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, posAccessor, 
        [&vertices]( fastgltf::math::fvec3 pos, size_t idx ) 
        {
            vertices[idx].pos = glm::vec3( pos.x(), pos.y(), pos.z() );
            vertices[idx].nrm = glm::vec3();
            vertices[idx].tex = glm::vec2();
        });
        m.UnMapVertexBuffer(index); 
        m.EnableVertexAttribute( index, 0, 3, GL_FLOAT, offsetof( Vertex, pos ) ); 

        auto* nrmIt = it->findAttribute("NORMAL");
        if ( nrmIt != it->attributes.end() )
        {
            const fastgltf::Accessor& nrmAccessor = m_asset.accessors[nrmIt->accessorIndex];
            vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) );
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, nrmAccessor, 
            [&vertices]( fastgltf::math::fvec3 nrm, size_t idx ) 
            {
                vertices[idx].nrm = glm::vec3( nrm.x(), nrm.y(), nrm.z() );
            });
            m.UnMapVertexBuffer(index); 
            m.EnableVertexAttribute( index, 1, 3, GL_FLOAT, offsetof( Vertex, nrm ) ); 
        }

        std::string texString = std::string("TEXCOORD_") + std::to_string(baseColorTexcoordIndex);
        const auto* texIt = it->findAttribute(texString); 
        if ( texIt != it->attributes.end() )
        {
            const fastgltf::Accessor& texAccessor = m_asset.accessors[texIt->accessorIndex];
            if ( !texAccessor.bufferViewIndex.has_value() )
                continue;

            vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) ); 
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>( m_asset, texAccessor, 
            [&vertices]( fastgltf::math::fvec2 uv, size_t idx )
            {
                vertices[idx].tex = glm::vec2( uv.x(), uv.y() );
            });
            m.UnMapVertexBuffer(index);
            m.EnableVertexAttribute( index, 2, 2, GL_FLOAT, offsetof( Vertex, tex) ); 
        }

        const fastgltf::Accessor& indexAccessor = m_asset.accessors[ it->indicesAccessor.value() ];
        if ( !indexAccessor.bufferViewIndex.has_value() )
        {
            m.Destroy();
            return std::unexpected( Error( "Mesh has no index buffer", Error::errorMinor ) );
        }

        if ( indexAccessor.componentType == fastgltf::ComponentType::UnsignedByte || indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort )
        {
            m.InitializeIndexBuffer( index, indexAccessor.count, GL_UNSIGNED_SHORT ); 
            uint16_t* indices = static_cast<uint16_t*>( m.MapIndexBuffer( index, GL_WRITE_ONLY ) );
            fastgltf::copyFromAccessor<uint16_t>( m_asset, indexAccessor, indices  );
            m.UnMapIndexBuffer(index); 
        }
        else
        {
            m.InitializeIndexBuffer( index, indexAccessor.count, GL_UNSIGNED_INT ); 
            uint32_t* indices = static_cast<uint32_t*>( m.MapIndexBuffer( index, GL_WRITE_ONLY ) );
            fastgltf::copyFromAccessor<uint32_t>( m_asset, indexAccessor, indices  );
            m.UnMapIndexBuffer(index); 
        }
        m.EnableIndexBuffer(index); 
    }

    m.InitializeDrawBuffer();

    return m;
}

[[nodiscard]] bool Parser::ReadScene() const
{
    for ( auto nodeIt = m_asset.nodes.begin(); nodeIt != m_asset.nodes.end(); ++nodeIt )
    {
        if ( nodeIt->meshIndex.has_value() )
        {
            auto expectMesh = ReadMesh( m_asset.meshes[ nodeIt->meshIndex.value() ] );
            if ( !expectMesh.has_value() )
            {
                Error e = expectMesh.error();
                Logger::LogLine( Logger::severityWarning, "Failed to read mesh {} in file {}. Error: {} Message: {}", 
                                 std::distance( m_asset.nodes.begin(), nodeIt ), m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
                continue;
            }

            Mesh* m = new Mesh( expectMesh.value() );
            m->m_ID = Node::NodeLibrary.push_back(m);
            m->m_parent = m->m_ID;
            m->LocalTransform( std::get<fastgltf::TRS>(nodeIt->transform) );
            m->m_children.insert( m->m_children.end(), nodeIt->children.begin(), nodeIt->children.end() );
        }

    }

    // For each node, assign its children the index of its parent
    std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
    []( Node* n )
    {
        std::for_each( n->m_children.begin(), n->m_children.end(),
        [&n]( uint64_t idx )
        {
            Node* child = Node::NodeLibrary[idx];
            child->m_parent = n->m_ID; 
        });
    });

    return true;
}


