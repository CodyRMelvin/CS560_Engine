cmake_minimum_required( VERSION 3.31 )

project( dependencies LANGUAGES C )

file( GLOB files "glad/*.c" "glad/*.h" )

add_library( ${PROJECT_NAME} ${files} )

target_include_directories( ${PROJECT_NAME} PRIVATE glad )

add_subdirectory(external/SDL)
add_subdirectory(external/glm)

include_directories(external/glm)
include_directories(external/fbxsdk/Linux/include)

find_package(OpenGL REQUIRED)
target_link_libraries( ${PROJECT_NAME} PRIVATE OpenGL::GL )
target_link_libraries( ${PROJECT_NAME} INTERFACE SDL3::SDL3 )
target_link_libraries( ${PROJECT_NAME} INTERFACE glm::glm )
