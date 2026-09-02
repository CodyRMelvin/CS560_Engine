cmake_minimum_required(VERSION 3.30.8)

file(GLOB sources "*.cpp" "*.c")

file(GLOB headers "*.h")

add_executable(${CMAKE_PROJECT_NAME} ${sources} ${headers})