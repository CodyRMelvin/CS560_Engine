//**************************************************************************************************
// File   : Parser.h
// Author : Cody Melvin
// Date   : 2026/10/09
// Brief  : Interface for class used to parse gltf and other files
//**************************************************************************************************

#pragma once
#ifndef PARSER_H
#define PARSER_H

#include "Common.h"
#include "Mesh.h"

#include "fastgltf/core.hpp"

typedef class Camera Camera;
typedef class Node3D Node3D;
typedef class Joint Joint;

class Parser
{
    public:
        Parser( std::string path, fastgltf::Options options );
        Parser( std::filesystem::path path, fastgltf::Options options  );
        Parser( Parser& ) = delete;
        ~Parser() = default;

        [[nodiscard]] std::expected< Mesh, Error > ReadMesh( const fastgltf::Mesh& mesh, bool isSkinned = false ) const;
        [[nodiscard]] std::expected< Camera, Error > ReadCamera( const fastgltf::Camera& cam ) const;
        [[nodiscard]] std::expected< Mesh::Skin, Error > ReadSkin( const fastgltf::Skin& skin ) const;
        [[nodiscard]] bool ReadScene() const;

        operator bool() const { return m_opened; }

    private:
        static fastgltf::Parser m_parser;

        std::filesystem::path m_filePath;
        fastgltf::Asset m_asset;
        fastgltf::GltfDataBuffer m_buffer;
        bool m_opened;

};


#endif
