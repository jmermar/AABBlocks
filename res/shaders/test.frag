#version 450

layout (location = 0) in vec3 inUv;

//output write
layout (location = 0) out vec4 outFragColor;

layout (set = 1, binding = 0) uniform sampler2DArray atlas;

void main() 
{
	//return red
	outFragColor = texture(atlas, inUv);
}
