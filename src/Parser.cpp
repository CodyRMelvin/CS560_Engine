//**************************************************************************************************
// File   : Parser.cpp
// Author : Cody Melvin
// Date   : 2026/10/09
// Brief  : Implementation of class used to parse files
//**************************************************************************************************

#include "Parser.h"
#include "Mesh.h"
#include "Camera.h"
#include "Application.h"
#include "Joint.h"
#include "Animation.h"

#include "Shader.h"
#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include "fastgltf/tools.hpp"

#include <expected>

Parser::Parser( std::string path, fastgltf::Options options, fastgltf::Extensions extensions )
    : m_parser(extensions)
    , m_filePath(path) 
    , m_asset() 
    , m_buffer()
    , m_opened(false)
{
    // Purely to check if file can be read
    auto expectedFile = fastgltf::MappedGltfFile::FromPath(path); 
    if ( expectedFile.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityWarning, "Failed to read gltf file from path {}, error: {}", 
                         path, fastgltf::getErrorMessage( expectedFile.error() ) );
        return;
    }

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

Parser::Parser( std::filesystem::path path, fastgltf::Options options, fastgltf::Extensions extensions  )
    : m_parser(extensions)
    , m_filePath(path) 
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

[[nodiscard]] std::expected< Mesh, Error > Parser::ReadMesh( const fastgltf::Mesh& mesh, bool isSkinned ) const
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

        m.InitializeVertexBuffer( index, posAccessor.count, sizeof(Vertex) ); 
        Vertex* vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) );
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, posAccessor, 
        [&vertices]( fastgltf::math::fvec3 pos, size_t idx ) 
        {
            vertices[idx].pos = glm::vec3( pos.x(), pos.y(), pos.z() );
            vertices[idx].nrm = glm::vec3();
            vertices[idx].tex = glm::vec2();
            vertices[idx].boneIds = glm::uvec4(0);
            vertices[idx].weights = glm::vec4(0);
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

        if (isSkinned)
        {
            const auto* jointIt = it->findAttribute("JOINTS_0"); 
            if ( jointIt != it->attributes.end() )
            {
                const fastgltf::Accessor& jointAccessor = m_asset.accessors[ jointIt->accessorIndex ];
                if ( !jointAccessor.bufferViewIndex.has_value() )
                    continue;


                vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) );
                fastgltf::iterateAccessorWithIndex<fastgltf::math::ivec4>( m_asset, jointAccessor, 
                [&vertices]( fastgltf::math::ivec4 joints, size_t idx )
                {
                    vertices[idx].boneIds = glm::ivec4( joints.x(), joints.y(), joints.z(), joints.w() );
                });
                m.UnMapVertexBuffer(index);
                m.EnableVertexAttributeI( index, 3, 4, GL_UNSIGNED_INT, offsetof( Vertex, boneIds ) ); 
            }

            const auto* weightIt = it->findAttribute("WEIGHTS_0");
            if ( weightIt != it->attributes.end() )
            {
                const fastgltf::Accessor& weightAccessor = m_asset.accessors[ weightIt->accessorIndex ];
                if ( !weightAccessor.bufferViewIndex.has_value() )
                    continue;


                vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) );
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>( m_asset, weightAccessor, 
                [&vertices]( fastgltf::math::fvec4 weights, size_t idx )
                {
                    vertices[idx].weights = glm::vec4( weights.x(), weights.y(), weights.z(), weights.w() );
                });
                m.UnMapVertexBuffer(index);
                m.EnableVertexAttribute( index, 4, 4, GL_FLOAT, offsetof( Vertex, weights ) ); 
            }
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

    return m;
}

[[nodiscard]] std::expected< Camera, Error > Parser::ReadCamera( const fastgltf::Camera& cam ) const
{
    // Index 0 is projection camera, we do not support orthographic
    if ( cam.camera.index() == 1 )
        return std::unexpected( Error( "Orthographic cameras are not supported", Error::errorMinor ) );

    fastgltf::Camera::Perspective camData = std::get<fastgltf::Camera::Perspective>(cam.camera);
    Camera newCam;
    newCam.SetViewingDimensions( camData.yfov, Application::Get().GetWindow().AspectRatio() )
          .SetPlaneDistances( camData.znear, camData.zfar.value() );

    return newCam;
}

[[nodiscard]] std::expected< Mesh::Skin, Error > Parser::ReadSkin( const fastgltf::Skin& skin ) const
{
    if ( !skin.inverseBindMatrices.has_value() )
        return std::unexpected( Error( "No index for matrix accessor", Error::errorMinor ) );

    Mesh::Skin s;
    const fastgltf::Accessor& matAccessor = m_asset.accessors[ skin.inverseBindMatrices.value() ];
    if ( !matAccessor.bufferViewIndex.has_value() )
        return std::unexpected( Error( "No index for matrix accessor", Error::errorMinor ) );

    fastgltf::iterateAccessorWithIndex<fastgltf::math::fmat4x4>( m_asset, matAccessor, 
    [&s]( fastgltf::math::fmat4x4 mat, size_t idx )
    {
        glm::mat4 m = glm::make_mat4( mat.data() );
        s.inverseBindMatrices.push_back(m);
    });
    
    s.joints.insert( s.joints.end(), skin.joints.begin(), skin.joints.end());

    if ( skin.skeleton.has_value() )
        s.skeleton = skin.skeleton.value();
    else
        s.skeleton = s.joints[0];

    const auto& AddJoints = 
    [ &s, this ]( this const auto& self, size_t index, size_t parent ) -> void
    {
        fastgltf::Node node = m_asset.nodes[index];
        Joint* j = new Joint( index, parent );
        j->LocalTransform( std::get<fastgltf::TRS>(node.transform) );
        j->m_children.insert( j->m_children.begin(), node.children.begin(), node.children.end() );

        Node::NodeLibrary[index] = j;
        for ( const size_t idx : j->m_children )
            self( idx, index );
    };

    AddJoints( s.skeleton, s.skeleton );

    for ( int i = 0; i < s.joints.size(); ++i )
    {
        Joint* j = dynamic_cast<Joint*>( Node::NodeLibrary[ s.joints[i] ] );
        if (!j)
            continue;
        j->matrixIndex = i;
    }/*
    
    s.skeleton = 3;
    s.joints.push_back(3);
    s.joints.push_back(4);
    s.joints.push_back(5);

    Joint* j3 = new Joint();
    j3->LocalTransform( Transform( glm::vec3{ 1, 1, 1 }, glm::vec3{ 0, 1, 0, }, glm::vec3{ 0, 1, 0 }, 0 ) );
    j3->m_children.push_back(4);
    j3->m_children.push_back(5);
    j3->m_ID = 3;
    j3->m_parent = 3;
    j3->matrixIndex = 3;
    Node::NodeLibrary[3] = j3;

    Joint* j4 = new Joint();
    j4->LocalTransform( Transform( glm::vec3{ 1, 1, 1 }, glm::vec3{ -1, -1, 0, }, glm::vec3{ 0, 1, 0 }, 0 ) );
    j4->m_ID = 4;
    j4->m_parent = 3;
    j4->matrixIndex = 4;
    Node::NodeLibrary[4] = j4;

    Joint* j5 = new Joint();
    j5->LocalTransform( Transform( glm::vec3{ 1, 1, 1 }, glm::vec3{ 1, -1, 0, }, glm::vec3{ 0, 1, 0 }, 0 ) );
    j5->m_ID = 5;
    j5->m_parent = 3;
    j5->matrixIndex = 5;
    Node::NodeLibrary[5] = j5;*/


    return s;
}

[[nodiscard]] std::expected< Animation, Error > Parser::ReadAnimation( const fastgltf::Animation& animation ) const
{
    int counter = 0;
    Animation anim;
    for ( const fastgltf::AnimationChannel& channel : animation.channels )
    {
        if ( !channel.nodeIndex.value() )
            return std::unexpected( Error( "No node index in animaation channel", Error::errorMinor ) );

        Animation::Channel c = {0};
        c.target = channel.nodeIndex.value();
        c.path = static_cast<Animation::Path>( static_cast<int>(channel.path) - 1 );

        const fastgltf::AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
        c.interpolation = static_cast<Animation::Interpolation>(sampler.interpolation);

        const fastgltf::Accessor& inputAccessor = m_asset.accessors[sampler.inputAccessor];
        const fastgltf::Accessor& outputAccessor = m_asset.accessors[sampler.outputAccessor];
        c.frames.resize(inputAccessor.count);

        fastgltf::iterateAccessorWithIndex<float>( m_asset, inputAccessor, 
        [&c]( float input, size_t idx )
        {
            c.frames[idx].time = input;
        }); 

        switch(c.path)
        {
            case Animation::pathTranslation:
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, outputAccessor, 
                [&c]( fastgltf::math::fvec3 v, size_t idx )
                {
                    c.frames[idx].output.emplace<Animation::pathTranslation>( v.x(), v.y(), v.z() ); 
                }); 
                break;
            case Animation::pathRotation:
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fquat>( m_asset, outputAccessor, 
                [&c]( fastgltf::math::fquat quat, size_t idx )
                {
                    c.frames[idx].output.emplace<Animation::pathRotation>( quat.x(),  quat.y(),  quat.z(),  quat.w() ); 
                }); 
                break;

            case Animation::pathScale:
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, outputAccessor, 
                [&c]( fastgltf::math::fvec3 v, size_t idx )
                {
                    c.frames[idx].output.emplace<Animation::pathScale>( v.x(), v.y(), v.z() ); 
                }); 
                break;

            default: break;
        }

        anim.AddChannel(c);
    }

    return anim;
}

[[nodiscard]] bool Parser::ReadScene() const
{
    for ( auto nodeIt = m_asset.nodes.begin(); nodeIt != m_asset.nodes.end(); ++nodeIt )
    {

        Node* newNode = nullptr;
        if ( nodeIt->meshIndex.has_value() )
        {
            bool hasSkin = nodeIt->skinIndex.has_value();
            auto expectMesh = ReadMesh( m_asset.meshes[ nodeIt->meshIndex.value() ], hasSkin );

            if ( !expectMesh.has_value() )
            {
                Error e = expectMesh.error();
                Logger::LogLine( Logger::severityWarning, "Failed to read mesh at node {} in file {}. Error: {} Message: {}", 
                                 std::distance( m_asset.nodes.begin(), nodeIt ), m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
                continue;
            }

            if (hasSkin)
            {
                auto expectSkin = ReadSkin( m_asset.skins[ nodeIt->skinIndex.value() ] );
                if ( !expectSkin.has_value() )
                {
                    Error e = expectSkin.error();
                    Logger::LogLine( Logger::severityWarning, "Failed to read skeleton at node {} in file {}. Error: {} Message: {}", 
                                     std::distance( m_asset.nodes.begin(), nodeIt ), m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
                    continue;
                }

                expectMesh.value().SetSkin( expectSkin.value() );

                Mesh& m = expectMesh.value();
                std::vector<size_t> indexBuffer;
                size_t pIndex = m.GetAnimationPrimitive();

                m.InitializeVertexArray( pIndex, GL_LINES );
                m.InitializeVertexBuffer( pIndex, expectSkin.value().joints.size(), sizeof(Vertex) ); 
                Vertex* vertices = static_cast<Vertex*>( m.MapVertexBuffer( pIndex, GL_WRITE_ONLY ) );
                for ( int i = 0; i < expectSkin.value().joints.size(); ++i )
                {
                    Joint* j = static_cast<Joint*>( Node::NodeLibrary[ expectSkin.value().joints[i] ] );
                    //vertices[i].pos = j->LocalTransform().Matrix()[3];
                    vertices[i].pos = j->GlobalTransformMatrix()[3];
                    vertices[i].nrm = glm::vec3( 1, 0, 0 );
                    vertices[i].tex = glm::vec2(0);
                    vertices[i].boneIds = glm::ivec4( 0, 0, 0, 0 );
                    vertices[i].weights = glm::vec4( 1, 0, 0, 0 );

                    for ( const size_t child : j->m_children )
                    {
                        auto childIt = std::find( expectSkin.value().joints.begin(), expectSkin.value().joints.end(), child );
                        if ( childIt == expectSkin.value().joints.end() )
                            continue;

                        indexBuffer.push_back(i);
                        indexBuffer.push_back( std::distance( expectSkin.value().joints.begin(), childIt ) );
                    }
                }

                m.UnMapVertexBuffer(pIndex);
                m.EnableVertexAttribute( pIndex, 0, 3, GL_FLOAT, offsetof( Vertex, pos ) ); 
                m.EnableVertexAttribute( pIndex, 1, 3, GL_FLOAT, offsetof( Vertex, nrm ) ); 
                m.EnableVertexAttribute( pIndex, 2, 2, GL_FLOAT, offsetof( Vertex, tex ) ); 
                m.EnableVertexAttributeI( pIndex, 3, 4, GL_UNSIGNED_INT, offsetof( Vertex, boneIds ) ); 
                m.EnableVertexAttribute( pIndex, 4, 4, GL_FLOAT, offsetof( Vertex, weights ) ); 

                m.InitializeIndexBuffer( pIndex, indexBuffer.size(), GL_UNSIGNED_INT );
                uint32_t* indices = static_cast<uint32_t*>( m.MapIndexBuffer( pIndex, GL_WRITE_ONLY ) );
                std::copy( indexBuffer.data(), indexBuffer.data() + indexBuffer.size(), indices );
                m.UnMapIndexBuffer(pIndex);
                m.EnableIndexBuffer(pIndex);
                expectMesh.value() = m;
            }

            newNode = new Mesh( expectMesh.value() );
        }
        else if ( nodeIt->cameraIndex.has_value() )
        {
            auto expectCam = ReadCamera( m_asset.cameras[ nodeIt->cameraIndex.value() ] ); 
            if ( !expectCam.has_value() )
            {
                Error e = expectCam.error();
                Logger::LogLine( Logger::severityWarning, "Failed to read cam at node {} in file {}. Error: {} Message: {}", 
                                 std::distance( m_asset.nodes.begin(), nodeIt ), m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
                continue;
            }

            newNode  = new Camera( expectCam.value() );
        }

        if (newNode)
        {
            newNode->m_ID = std::distance( m_asset.nodes.begin(), nodeIt );
            Node::NodeLibrary[newNode->m_ID] = newNode;
            newNode->m_parent = newNode->m_ID;
            newNode->m_children.insert(newNode->m_children.end(), nodeIt->children.begin(), nodeIt->children.end() );


            Node3D* derived = dynamic_cast<Node3D*>(newNode); 
            if (derived)
                derived->LocalTransform( std::get<fastgltf::TRS>(nodeIt->transform) );
        }
    }

    for ( const fastgltf::Animation& animation : m_asset.animations )
    {
        auto expectAnim = ReadAnimation(animation);

        if ( !expectAnim.has_value() )
        {
            Error e = expectAnim.error();
            Logger::LogLine( Logger::severityWarning, "Failed to read animation {} in file {}. Error: {} Message: {}", 
                             animation.name, m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
            continue;
        }

        Animation::AnimationLibrary.push_back( expectAnim.value() );
    }

    // For each node, assign its children the index of its parent
    std::for_each( Node::NodeLibrary.begin(), Node::NodeLibrary.end(),
    []( auto& pair )
    {
        std::for_each( pair.second->m_children.begin(), pair.second->m_children.end(),
        [&pair]( uint64_t idx )
        {
            Node* child = Node::NodeLibrary[idx];
            child->m_parent = pair.second->m_ID; 
        });
    });

    return true;
}


