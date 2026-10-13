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

#include "fastgltf/core.hpp"

typedef class Mesh Mesh;
typedef class Mesh2 Mesh2;

class Parser
{
    public:
        Parser( std::string path, fastgltf::Options options );
        Parser( std::filesystem::path path, fastgltf::Options options  );
        Parser( Parser& ) = delete;
        ~Parser() = default;

        [[nodiscard]] std::expected< Mesh, Error > ReadMesh( const fastgltf::Mesh& mesh ) const;
        [[nodiscard]] bool ReadMeshes( std::vector<Mesh>& ) const;

        operator bool() const { return m_opened; }

    private:
        static fastgltf::Parser m_parser;

        std::filesystem::path m_filePath;
        fastgltf::Asset m_asset;
        fastgltf::GltfDataBuffer m_buffer;
        bool m_opened;

};


#endif
