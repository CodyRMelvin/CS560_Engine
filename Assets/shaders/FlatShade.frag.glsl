#version 460 core

layout (location = 0) in vec3 cameraNormal;
out vec4 color;

void main ()
{
    vec3 n = normalize(cameraNormal);
    float shade = max( max(0, n.z), max(n.x, n.y) );
    color = vec4( n, 1 );
}