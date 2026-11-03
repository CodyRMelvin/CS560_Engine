cmake_minimum_required( VERSION 3.31 )

project(DearImGUI)
add_library( ${PROJECT_NAME} STATIC
    ${PROJECT_SOURCE_DIR}/imgui.cpp
    ${PROJECT_SOURCE_DIR}/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/imgui_widgets.cpp

    ${PROJECT_SOURCE_DIR}/imgui_impl_sdl3.cpp
    ${PROJECT_SOURCE_DIR}/imgui_impl_opengl3.cpp
)

target_link_libraries( ${PROJECT_NAME} PUBLIC SDL3::SDL3 )

target_include_directories( ${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/
)
