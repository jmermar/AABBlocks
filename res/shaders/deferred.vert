#version 450
#extension GL_EXT_buffer_reference : require

layout(set = 0, binding = 0) readonly uniform CameraData {
    mat4 proj;
    mat4 view;
    mat4 projView;
	vec4 planes[6];
} ubo;

layout(set = 1, binding = 4) readonly buffer Quad {
    vec3 vertices[];
} v;

layout(location = 0) out vec2 outUv;


void main() 
{
	uint idx = gl_VertexIndex;

	outUv = v.vertices[idx].xy;

	gl_Position = vec4(v.vertices[idx].xy * 2 - vec2(1), 0, 1);
}
