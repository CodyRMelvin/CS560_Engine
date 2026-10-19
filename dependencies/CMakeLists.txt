cmake_minimum_required( VERSION 3.31 )

project( dependencies LANGUAGES C )

file( GLOB files "glad/*.c" "glad/*.h" "external/SIV/index_vector.hpp" )

set( CMAKE_CXX_FLAGS_DEBUG "-O0 -g" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O4 -g" )
set( CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG" )

add_library( ${PROJECT_NAME} ${files} )

target_include_directories( ${PROJECT_NAME} PRIVATE glad )
target_include_directories( ${PROJECT_NAME} PRIVATE external/SIV )

add_subdirectory(external/SDL)
add_subdirectory(external/glm)
add_subdirectory(external/fastgltf)

target_link_libraries( ${PROJECT_NAME} INTERFACE SDL3::SDL3 )
target_link_libraries( ${PROJECT_NAME} INTERFACE glm::glm )
target_link_libraries( ${PROJECT_NAME} INTERFACE fastgltf )

include_directories(external/glm)

find_package(OpenGL REQUIRED)
target_link_libraries( ${PROJECT_NAME} PRIVATE OpenGL::GL )