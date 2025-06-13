#version 450

layout (location = 0) in vec3 inUv;
layout (location=1) in vec3 outNorm;

//output write
layout (location = 0) out vec4 outFragColor;

layout (set = 1, binding = 0) uniform sampler2DArray atlas;

void main() 
{
	float lambert = dot(normalize(vec3(2, 3, 3)), outNorm) * 1.5;
	float light = max(0.4, min(1, lambert));
	//return red
	outFragColor = texture(atlas, inUv) * light;
}
