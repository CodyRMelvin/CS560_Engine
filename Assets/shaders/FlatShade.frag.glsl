#version 460 core

layout (location = 0) in vec3 cameraNormal;
out vec4 color;

void main ()
{
    vec3 n = normalize(cameraNormal);
    float shade = max( 0, n.z );
    color = vec4( shade, 0, 0, 1 );
}