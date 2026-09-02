cmake_minimum_required(VERSION 3.31)

set( CMAKE_CXX_COMPILER clang++ )
set( CMAKE_C_COMPILER clang )

project(CS560 LANGUAGES C CXX)

add_subdirectory(src)
add_subdirectory(dependencies)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin/)

target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:_DEBUG>")

set( CMAKE_CXX_STANDARD 23 )
target_compile_features( ${PROJECT_NAME} PUBLIC cxx_std_23 )

set( CMAKE_CXX_FLAGS "-Wall -Werror -Wextra" )
set( CMAKE_CXX_FLAGS_DEBUG "-g" )
set( CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG" )
# set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

target_include_directories( ${PROJECT_NAME} PRIVATE dependencies )
target_link_libraries( ${PROJECT_NAME} PRIVATE dependencies )

add_custom_target(
    run
    COMMAND ${PROJECT_NAME}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DEPENDS ${PROJECT_NAME}
    USES_TERMINAL
)
