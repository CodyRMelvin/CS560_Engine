cmake_minimum_required( VERSION 3.31 )

project( external LANGUAGES C )

file( GLOB ufbxFiles "ufbx/ufbx.c" "ufbx/ufbx.h" )
add_library( ${PROJECT_NAME} ${ufbxFiles} )
target_include_directories( ${PROJECT_NAME} PRIVATE ufbx )

add_subdirectory(SDL)
add_subdirectory(glm)

find_package(OpenGL REQUIRED)
target_link_libraries( ${PROJECT_NAME} PRIVATE OpenGL::GL )
target_link_libraries( ${PROJECT_NAME} INTERFACE SDL3::SDL3 )
target_link_libraries( ${PROJECT_NAME} INTERFACE glm::glm )
