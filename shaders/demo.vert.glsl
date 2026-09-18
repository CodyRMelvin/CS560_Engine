#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec2 tex;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 perspective;

void main ()
{
    gl_Position = perspective * view * model * vec4( pos, 1.0 );
}