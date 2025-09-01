#version 450

layout(location = 0) in vec3 inUv;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec3 inPos;
layout(location = 3) in mat3 inTBN;

//output write
layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMaterial;
layout(location = 3) out vec4 outPos;

layout(set = 1,
	   binding = 0) uniform sampler2DArray atlas;
layout(set = 1,
	   binding = 3) uniform sampler2DArray normal;
layout(set = 1, binding = 4) uniform
	sampler2DArray material;

void main()
{
	vec3 normal = normalize(
		inTBN * (texture(normal, inUv).xyz * 2 -
				 vec3(1)));

	outAlbedo = texture(atlas, inUv);
	outNormal = vec4(normal * 0.5 + vec3(0.5), 1);
	outMaterial = texture(material, inUv);
	outPos = vec4(inPos, 0);
}
