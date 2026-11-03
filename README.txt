Reository located at https://github.com/CodyRMelvin/CS560_Engine

Project is built using cmake and ninja multiconfig
To build from scratch on windows, run the cmake_windows.bat file
On linux, run the build_cmake script

To compile and run on windows, locate the build folder, within is a visual studio solution. Make sure to set CS560 as the startup project
To compile and run on linux, use the build_program and run scripts, passing in the configuration (debug, release, relwithdebuginfo). These are not case sensitive

Project is made with C++23 and the clang compiler on linux, msvc on windows, with Glad, OpenGL, DearImGUI, SDL, fastgltf (which includes simdjson and simdutf), and glm.

This project renders skinned meshes and skeletal animation using opengl and gltf via indirect rendering. Lerp and spherical lerp are implemented as the only interpolation techniques.
The scene is handled in a node-based tree hierarchy, stored in an unordered map. Seperable programs and program pipelines are utilized to decouple shaders and allow for more modular rendering pipelines
Gltf is used as the data format. Files are sourced from the fbx files provided in the framework, converted to gltf with https://github.com/facebookincubator/FBX2glTF and tweaked/cleaned in blender.
Animating translations, rotations, and scales through lerping and spherical lerping is supported. The frame work for other interpolation techniques and weight animations is present, but not utilised.

Controls:
    Camera forwards: W
    Camera backwards: D
    Camera left: A
    Camera right: D
    Camera up: Q
    Camera down: E
    Tilt camera down: down arrow
    Tilt camera up: up arrow
    Turn camera left: left arrow
    Turn camera right: right arrow
    Pause/play animations: space bar
    Exit program: ESC

A gui is provided to allow for switching between gltf files, switching betwwen animations (if multiple are present), pausing/playing the animations, and toggling rendering settings

