#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex;
layout (location = 3) in uvec4 boneIDs;
layout (location = 4) in vec4 weights;

layout (location = 0) uniform mat4 modelView;
layout (location = 1) uniform mat4 perspective;
layout (location = 2) uniform mat4 normalMatrix;
layout (location = 3) uniform mat4 boneMatrices[100];

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

layout (location = 0) out vec3 cameraNormal;

void main ()
{
    mat4 boneTransform = 
        weights.x * boneMatrices[boneIDs.x] +
        weights.y * boneMatrices[boneIDs.y] +
        weights.z * boneMatrices[boneIDs.z] +
        weights.w * boneMatrices[boneIDs.w]; 

    gl_Position = perspective * modelView * boneTransform * vec4( pos, 1.f );
    cameraNormal = mat3(normalMatrix) * normal;
}