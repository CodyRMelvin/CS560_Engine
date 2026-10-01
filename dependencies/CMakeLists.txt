cmake_minimum_required( VERSION 3.31 )

project( dependencies LANGUAGES C )

file( GLOB gladFiles "glad/*.c" "glad/*.h" )
add_library( ${PROJECT_NAME} ${gladFiles} )
target_include_directories( ${PROJECT_NAME} PRIVATE glad )

add_subdirectory(external)
target_include_directories( ${PROJECT_NAME} PRIVATE external )
target_link_libraries( ${PROJECT_NAME} PRIVATE external )
