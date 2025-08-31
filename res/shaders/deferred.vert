#version 450
#extension GL_EXT_buffer_reference : require

layout(set = 0, binding = 0) readonly uniform CameraData {
    mat4 proj;
    mat4 view;
    mat4 projView;
    mat4 iProjViewMatrix;
	mat4 iViewMatrix;
	mat4 iProjMatrix;
	vec4 planes[6];
	vec3 cameraPosition;
    float ambientLight;
    vec3 lightDirection;
    float lightIntensity;
    float exposure;
} ubo;

layout(set = 1, binding = 4) readonly buffer Quad {
    vec3 vertices[];
} v;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outRay;


void main() 
{
	uint idx = gl_VertexIndex;

	outUv = v.vertices[idx].xy;
    vec4 ray = ubo.iProjMatrix * vec4(v.vertices[idx].xy * 2 - 1, 1, 1);
    outRay = ray.xyz / ray.w;
    outRay = (ubo.iViewMatrix * vec4(outRay, 0)).xyz;

	gl_Position = vec4(v.vertices[idx].xy * 2 - vec2(1), 0, 1);
}
