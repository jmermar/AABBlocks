#version 450

layout (location = 0) in vec2 inUv;

layout (set = 1, binding = 0) uniform sampler2D color;
layout (set = 1, binding = 1) uniform sampler2D normal;
layout (set = 1, binding = 2) uniform sampler2D pos;
layout (set = 1, binding = 3) uniform sampler2D material;

//output write
layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler2DArray atlas;

void main() 
{
	float il = clamp(dot(texture(normal, inUv).xyz, normalize(vec3(-1, 2, 1))), 0.3, 1);
	outColor = texture(color, inUv) * il;
}
