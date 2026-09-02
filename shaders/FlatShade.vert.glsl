#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 perspective;
layout (location = 3) uniform mat4 normalMatrix;
layout (location = 0) out vec3 cameraNormal;

void main ()
{
    gl_Position = perspective * view * model * vec4( pos, 1.0 );
    cameraNormal = mat3(view) * mat3(normalMatrix) * normal;
}