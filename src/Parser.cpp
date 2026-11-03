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

    // Load data buffer from passed in file
    auto expectedBuf = fastgltf::GltfDataBuffer::FromPath(m_filePath);
    if ( expectedBuf.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityWarning, "Failed to read gltf buffer from path {}, error: {}", 
                         path, fastgltf::getErrorMessage( expectedBuf.error() ) );
        return;
    }

    // Move said data into parser
    m_buffer = std::move( expectedBuf.get() );
    // Load the gltf asset from the buffer
    auto expectedAsset = m_parser.loadGltf( m_buffer, m_filePath.parent_path(), options );
    if ( expectedAsset.error() != fastgltf::Error::None )
    {
        Logger::LogLine( Logger::severityWarning, "Failed to load gltf asset from path {}, error: {}", 
                         path, fastgltf::getErrorMessage( expectedAsset.error() ) );
        return;
    }

    // Move asset data into parser
    m_asset = std::move( expectedAsset.get() );
    // State the file was successfully opened
    m_opened = true;
}

Parser::Parser( std::filesystem::path path, fastgltf::Options options, fastgltf::Extensions extensions  )
    : m_parser(extensions)
    , m_filePath(path) 
    , m_asset() 
    , m_buffer()
    , m_opened(false)
{
    // Same operations as prior constructor but utilizing a filesystem::path object isntead of a string
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
    // Create a mesh object with the same number of primitives
    Mesh m( mesh.primitives.size() );
    // Perform the following operations for each primitive in the gltf mesh
    for ( auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it )
    {
        // Get the iterator pointing to the POSITION attribute
        const fastgltf::Attribute* posIt = it->findAttribute("POSITION");

        // Position attribute is required
        if ( posIt == it->attributes.end() )
            return std::unexpected( Error( "No position attribute found in mesh", Error::errorMinor ) );
        // An indices accessor is required
        if ( !it->indicesAccessor.has_value() )
            return std::unexpected( Error( "No indices accessor found in mesh", Error::errorMinor ) );

        // Get the index of the primitive currently being created
        uint16_t index = std::distance( mesh.primitives.begin(), it );

        // index of where the primitives texture coordinates are stored
        size_t baseColorTexcoordIndex = 0;
        // Perofrm the following only if the gltf mesh has a material
        if ( it->materialIndex.has_value() )
        {
            // Get the material
            const fastgltf::Material& material = m_asset.materials[ it->materialIndex.value() ];

            // Get the color of the base texture
            auto& baseColorTexture = material.pbrData.baseColorTexture;
            // Perform the following only if said texture exists
            if ( baseColorTexture.has_value() )
            {
                // Get the texture proper
                const fastgltf::Texture& texture = m_asset.textures[baseColorTexture->textureIndex];
                // Image index is required
                if ( !texture.imageIndex.has_value() )
                    return std::unexpected( Error( "Mesh has no image index", Error::errorMinor ) );

                // Get the index of the texture coordinates
                if ( baseColorTexture->transform && baseColorTexture->transform->texCoordIndex.has_value() )
                    baseColorTexcoordIndex = baseColorTexture->transform->texCoordIndex.value();
                else
                    baseColorTexcoordIndex = material.pbrData.baseColorTexture->texCoordIndex;
            }
        }

        // Create the primitive's vertex array
        m.InitializeVertexArray( index, fastgltf::to_underlying(it->type) ); 
        // Get accessor for primitive vertex positions
        // In GLTF, an accessor is information on how to interperet and access buffer data
        const fastgltf::Accessor& posAccessor = m_asset.accessors[posIt->accessorIndex];
        // A buffer view is required
        // in GLTF, a buffer view is information on where the buffer is and how it's data is layed out
        if ( !posAccessor.bufferViewIndex.has_value() )
            continue;

        // Create the primitive's vertex buffer
        m.InitializeVertexBuffer( index, posAccessor.count, sizeof(Vertex) ); 
        // Map the buffer to the cpu
        Vertex* vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) );
        // Iterate through buffer data 
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>( m_asset, posAccessor, 
        [&vertices]( fastgltf::math::fvec3 pos, size_t idx ) 
        {
            // Initialize vertex data to 0, pass in position data
            vertices[idx].pos = glm::vec3( pos.x(), pos.y(), pos.z() );
            vertices[idx].nrm = glm::vec3();
            vertices[idx].tex = glm::vec2();
            vertices[idx].boneIds = glm::uvec4(0);
            vertices[idx].weights = glm::vec4(0);
        });
        // Retrun the buffer data and enable the position attribute
        m.UnMapVertexBuffer(index); 
        m.EnableVertexAttribute( index, 0, 3, GL_FLOAT, offsetof( Vertex, pos ) ); 

        // Get an iterator pointing to the NORMAL attribute
        auto* nrmIt = it->findAttribute("NORMAL");
        if ( nrmIt != it->attributes.end() )
        {
            // Same operations as with POSITION accessor, but for vertex normals
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

        // Get an iterator pointing to TEXCOORD_X accessor, where X is the previously acquired baseColorTexcoordIndex
        std::string texString = std::string("TEXCOORD_") + std::to_string(baseColorTexcoordIndex);
        const auto* texIt = it->findAttribute(texString); 
        if ( texIt != it->attributes.end() )
        {
            // Texcoord accessor is required
            const fastgltf::Accessor& texAccessor = m_asset.accessors[texIt->accessorIndex];
            if ( !texAccessor.bufferViewIndex.has_value() )
                continue;

            // Same operations as prior but with vertex texcoords
            vertices = static_cast<Vertex*>( m.MapVertexBuffer( index, GL_WRITE_ONLY ) ); 
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>( m_asset, texAccessor, 
            [&vertices]( fastgltf::math::fvec2 uv, size_t idx )
            {
                vertices[idx].tex = glm::vec2( uv.x(), uv.y() );
            });
            m.UnMapVertexBuffer(index);
            m.EnableVertexAttribute( index, 2, 2, GL_FLOAT, offsetof( Vertex, tex) ); 
        }

        // If this mesh is skinned, then perform the prior operations again but for vertex weights and affected joint indices
        if (isSkinned)
        {
            const auto* jointIt = it->findAttribute("JOINTS_0"); 
            if ( jointIt != it->attributes.end() )
            {
                // Joint accessor is required
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
                // weights accessor is required
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

        // Get iterator to indices accessor
        const fastgltf::Accessor& indexAccessor = m_asset.accessors[ it->indicesAccessor.value() ];
        // If not found, destroy the mesh and exit
        if ( !indexAccessor.bufferViewIndex.has_value() )
        {
            m.Destroy();
            return std::unexpected( Error( "Mesh has no index buffer", Error::errorMinor ) );
        }

        // if the indices are stored via a small data type, use GL_UNSIGNED_SHORT
        if ( indexAccessor.componentType == fastgltf::ComponentType::UnsignedByte || indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort )
        {
            // Create the index buffer, map it's data to the cpu, and copy the accessor data directly into it
            m.InitializeIndexBuffer( index, indexAccessor.count, GL_UNSIGNED_SHORT ); 
            uint16_t* indices = static_cast<uint16_t*>( m.MapIndexBuffer( index, GL_WRITE_ONLY ) );
            fastgltf::copyFromAccessor<uint16_t>( m_asset, indexAccessor, indices  );
            m.UnMapIndexBuffer(index); 
        }
        // Otherwise use GL_UNSIGNED_INT
        else
        {
            // Same operation as prior
            m.InitializeIndexBuffer( index, indexAccessor.count, GL_UNSIGNED_INT ); 
            uint32_t* indices = static_cast<uint32_t*>( m.MapIndexBuffer( index, GL_WRITE_ONLY ) );
            fastgltf::copyFromAccessor<uint32_t>( m_asset, indexAccessor, indices  );
            m.UnMapIndexBuffer(index); 
        }
        // Bind the index buffer to the VAO
        m.EnableIndexBuffer(index); 
    }

    return m;
}

[[nodiscard]] std::expected< Camera, Error > Parser::ReadCamera( const fastgltf::Camera& cam ) const
{
    // Index 0 is perspective camera, we do not support orthographic
    if ( cam.camera.index() == 1 )
        return std::unexpected( Error( "Orthographic cameras are not supported", Error::errorMinor ) );

    // Get the gltf camera data and copy it into a camera node
    fastgltf::Camera::Perspective camData = std::get<fastgltf::Camera::Perspective>(cam.camera);
    Camera newCam;
    newCam.SetViewingDimensions( camData.yfov, Application::Get().GetWindow().AspectRatio() )
          .SetPlaneDistances( camData.znear, camData.zfar.value() );

    return newCam;
}

[[nodiscard]] std::expected< Mesh::Skin, Error > Parser::ReadSkin( const fastgltf::Skin& skin ) const
{
    // Inverse bind matrices are required
    if ( !skin.inverseBindMatrices.has_value() )
        return std::unexpected( Error( "No index for matrix accessor", Error::errorMinor ) );

    Mesh::Skin s;
    // The buffer view index is required
    const fastgltf::Accessor& matAccessor = m_asset.accessors[ skin.inverseBindMatrices.value() ];
    if ( !matAccessor.bufferViewIndex.has_value() )
        return std::unexpected( Error( "No index for matrix accessor", Error::errorMinor ) );

    // Iterate through each matrix and add it to the skin's array of inverse bind matrices
    fastgltf::iterateAccessorWithIndex<fastgltf::math::fmat4x4>( m_asset, matAccessor, 
    [&s]( fastgltf::math::fmat4x4 mat, size_t idx )
    {
        glm::mat4 m = glm::make_mat4( mat.data() );
        s.inverseBindMatrices.push_back(m);
    });

    // Copy over the array of joint indices
    s.joints.insert( s.joints.end(), skin.joints.begin(), skin.joints.end());

    // Assign skeleton's root node value is available
    if ( skin.skeleton.has_value() )
        s.skeleton = skin.skeleton.value();

    return s;
}

[[nodiscard]] std::expected< Animation, Error > Parser::ReadAnimation( const fastgltf::Animation& animation ) const
{
    int counter = 0;
    Animation anim;
    for ( const fastgltf::AnimationChannel& channel : animation.channels )
    {
        // Index of affected is required
        if ( !channel.nodeIndex.has_value() )
            return std::unexpected( Error( "No node index in animation channel", Error::errorMinor ) );

        // Create an annimation channel and copy over node index and path
        Animation::Channel c = {0};
        c.target = channel.nodeIndex.value();
        c.path = static_cast<Animation::Path>( static_cast<int>(channel.path) - 1 );

        // Get the sampler and pass ther interpolation method into channel
        const fastgltf::AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
        c.interpolation = static_cast<Animation::Interpolation>(sampler.interpolation);

        // Get accessors for path's inputs and outputs
        // In GLTF a path input is the time of the keyframe, and output is the new value for the transform or weights
        const fastgltf::Accessor& inputAccessor = m_asset.accessors[sampler.inputAccessor];
        const fastgltf::Accessor& outputAccessor = m_asset.accessors[sampler.outputAccessor];
        // Preallocate and initialize the number of frames
        c.frames.resize(inputAccessor.count);

        // Copy over the input time values into the keyframe array
        fastgltf::iterateAccessorWithIndex<float>( m_asset, inputAccessor, 
        [&c]( float input, size_t idx )
        {
            c.frames[idx].time = input;
        }); 

        // Copy over the ouput values
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

        // Add the new channel to the animation
        anim.AddChannel(c);
    }

    // Copy over the animations name, if no name is present default to "Animation"
    anim.Name( animation.name.c_str() );
    if ( anim.Name().empty() )
        anim.Name("Animation");

    return anim;
}

[[nodiscard]] bool Parser::ReadScene() const
{
    // Iterate through each node in the scene heirarchy
    for ( auto nodeIt = m_asset.nodes.begin(); nodeIt != m_asset.nodes.end(); ++nodeIt )
    {

        Node* newNode = nullptr;
        // Read in a mesh if current node has one
        if ( nodeIt->meshIndex.has_value() )
        {
            // Check if mesh is skinned
            bool hasSkin = nodeIt->skinIndex.has_value();
            // Read in the mesh
            auto expectMesh = ReadMesh( m_asset.meshes[ nodeIt->meshIndex.value() ], hasSkin );

            // Throw error if mesh reading fails
            if ( !expectMesh.has_value() )
            {
                Error e = expectMesh.error();
                Logger::LogLine( Logger::severityWarning, "Failed to read mesh at node {} in file {}. Error: {} Message: {}", 
                                 std::distance( m_asset.nodes.begin(), nodeIt ), m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
                continue;
            }

            // Read in a skin if the mesh is skinned
            if (hasSkin)
            {
                auto expectSkin = ReadSkin( m_asset.skins[ nodeIt->skinIndex.value() ] );
                // Throw error id reading fails
                if ( !expectSkin.has_value() )
                {
                    Error e = expectSkin.error();
                    Logger::LogLine( Logger::severityWarning, "Failed to read skeleton at node {} in file {}. Error: {} Message: {}", 
                                     std::distance( m_asset.nodes.begin(), nodeIt ), m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
                    continue;
                }

                // Add skin to mesh
                expectMesh.value().SetSkin( expectSkin.value() );

                // Set up animation primitive index
                Mesh& m = expectMesh.value();
                std::vector<size_t> indexBuffer;
                size_t pIndex = m.GetAnimationPrimitive();

                // Initialize vao and vbo for animation skeleton mesh
                m.InitializeVertexArray( pIndex, GL_LINES );
                m.InitializeVertexBuffer( pIndex, expectSkin.value().joints.size(), sizeof(Vertex) ); 
                // Map vertex data to cpu
                Vertex* vertices = static_cast<Vertex*>( m.MapVertexBuffer( pIndex, GL_WRITE_ONLY ) );
                // Iterate through each joint in the skin
                for ( int i = 0; i < expectSkin.value().joints.size(); ++i )
                {
                    // Get the node of the current joint
                    fastgltf::Node node = m_asset.nodes[ expectSkin.value().joints[i] ];

                    // Copy it's translation into the vertex position data, initialize rest to default values
                    float* data = std::get<fastgltf::TRS>( node.transform ).translation.data();
                    vertices[i].pos = glm::vec3( data[0], data[1], data[2] );
                    vertices[i].nrm = glm::vec3( 1, 0, 0 );
                    vertices[i].tex = glm::vec2(0);
                    vertices[i].boneIds = glm::ivec4( i, 0, 0, 0 );
                    vertices[i].weights = glm::vec4( 1, 0, 0, 0 );

                    // Add to index buffer this joint paired with each of it's children, so that a line is drawn between each joint pair
                    for ( const size_t child : node.children )
                    {
                        auto childIt = std::find( expectSkin.value().joints.begin(), expectSkin.value().joints.end(), child );
                        if ( childIt == expectSkin.value().joints.end() )
                            continue;

                        indexBuffer.push_back(i);
                        indexBuffer.push_back( std::distance( expectSkin.value().joints.begin(), childIt ) );
                    }
                }

                // Return vertex data to gpu, enable the attributes
                m.UnMapVertexBuffer(pIndex);
                m.EnableVertexAttribute( pIndex, 0, 3, GL_FLOAT, offsetof( Vertex, pos ) ); 
                m.EnableVertexAttribute( pIndex, 1, 3, GL_FLOAT, offsetof( Vertex, nrm ) ); 
                m.EnableVertexAttribute( pIndex, 2, 2, GL_FLOAT, offsetof( Vertex, tex ) ); 
                m.EnableVertexAttributeI( pIndex, 3, 4, GL_UNSIGNED_INT, offsetof( Vertex, boneIds ) ); 
                m.EnableVertexAttribute( pIndex, 4, 4, GL_FLOAT, offsetof( Vertex, weights ) ); 

                // Create the index buffer
                m.InitializeIndexBuffer( pIndex, indexBuffer.size(), GL_UNSIGNED_INT );
                // Map buffer to cpu and copy in data
                uint32_t* indices = static_cast<uint32_t*>( m.MapIndexBuffer( pIndex, GL_WRITE_ONLY ) );
                std::copy( indexBuffer.data(), indexBuffer.data() + indexBuffer.size(), indices );
                // Return buffer and enable
                m.UnMapIndexBuffer(pIndex);
                m.EnableIndexBuffer(pIndex);
                // Update expectMesh value 
                expectMesh.value() = m;
            }

            // Allocate a new mesh using the read values
            newNode = new Mesh( expectMesh.value() );
        }
        // If the node has a camera read that in
        else if ( nodeIt->cameraIndex.has_value() )
        {
            // Read in camera and log error if read is unsuccessful
            auto expectCam = ReadCamera( m_asset.cameras[ nodeIt->cameraIndex.value() ] ); 
            if ( !expectCam.has_value() )
            {
                Error e = expectCam.error();
                Logger::LogLine( Logger::severityWarning, "Failed to read cam at node {} in file {}. Error: {} Message: {}", 
                                 std::distance( m_asset.nodes.begin(), nodeIt ), m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
                continue;
            }

            // Allocate camera node
            newNode  = new Camera( expectCam.value() );
        }

        // Get the index of the current node
        uint64_t idx = std::distance( m_asset.nodes.begin(), nodeIt );
        // If new node hasn't been allocated yet, grab it from the library if it was allocated elsewhere
        // otherwise allocate a basic Node3D now
        if (!newNode)
        {
            if ( Node::NodeLibrary.contains(idx) )
                newNode = Node::NodeLibrary[idx];
            if (!newNode)
                newNode = new Node3D;
        }

        // Assign node's ID
        newNode->m_ID = idx;
        // Add node to library
        Node::NodeLibrary[newNode->m_ID] = newNode;
        // Default parent id to current id
        newNode->m_parent = newNode->m_ID;
        // Copy over child id's from gltf
        newNode->m_children.insert(newNode->m_children.end(), nodeIt->children.begin(), nodeIt->children.end() );


        // If this is a Node3D, set it's local transform
        Node3D* derived = dynamic_cast<Node3D*>(newNode); 
        if (derived)
            derived->LocalTransform( std::get<fastgltf::TRS>(nodeIt->transform) );
    }

    // Read in each animation from the asset
    for ( const fastgltf::Animation& animation : m_asset.animations )
    {
        // Read in the animation
        auto expectAnim = ReadAnimation(animation);

        // Log error if reading fails
        if ( !expectAnim.has_value() )
        {
            Error e = expectAnim.error();
            Logger::LogLine( Logger::severityWarning, "Failed to read animation {} in file {}. Error: {} Message: {}", 
                             animation.name, m_filePath.string(), Error::TypeString( e.GetType() ), e.What() );
            continue;
        }

        // Add to animation library
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
            if (child)
                child->m_parent = pair.second->m_ID; 
        });
    });

    return true;
}


