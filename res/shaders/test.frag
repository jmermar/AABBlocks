#version 450

layout (location = 0) in vec3 inUv;
layout (location=1) in vec3 inNorm;
layout (location=2) in vec3 inPos;

//output write
layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outMaterial;
layout (location = 3) out vec4 outPos;

layout (set = 1, binding = 0) uniform sampler2DArray atlas;

void main() 
{
	outAlbedo = texture(atlas, inUv);
	outNormal = vec4(inNorm, 0);
	outMaterial = vec4(1);
	outPos = vec4(inPos, 0);
}
