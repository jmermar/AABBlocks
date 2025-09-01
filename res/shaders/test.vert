#version 450
#extension GL_EXT_buffer_reference : require
#include "includes/globalData.h"

layout(location = 0) out vec3 outUv;
layout(location = 1) out vec3 outNorm;
layout(location = 2) out vec3 outPos;
layout(location = 3) out mat3 outTBN;

struct ChunkFace
{
	uint posAndFace;
	uint textureId;
};

struct Vertex
{
	vec3 pos;
	vec3 norm;
	vec3 tangent;
	vec3 bitangent;
	vec2 uv;
};

layout(std430,
	   buffer_reference,
	   buffer_reference_align =
		   8) readonly buffer FacesAddr
{
	ChunkFace faces[];
};

struct ChunkData
{
	FacesAddr faces;
	vec4 position;
	uint vertexCount;
};

layout(std430,
	   set = 1,
	   binding = 1) readonly buffer VertexBuffer
{
	Vertex vertices[];
};

layout(std430,
	   set = 1,
	   binding = 2) readonly buffer Chunks
{
	ChunkData chunks[];
}
chunks;

void main()
{
	uint idx = gl_VertexIndex;
	uint fineIdx = idx % 6;
	ChunkData chunk =
		chunks.chunks[gl_InstanceIndex];

	// Interpretar el address como un puntero a un array de VertexBuffer
	ChunkFace face = chunk.faces.faces[idx / 6];
	uint faceId = face.posAndFace >> 24;
	float x = face.posAndFace & 0xff;
	float y = (face.posAndFace >> 8) & 0xff;
	float z = (face.posAndFace >> 16) & 0xff;
	Vertex vertex =
		vertices[faceId * 6 + fineIdx];

	vec3 T = vertex.tangent;
	vec3 B = vertex.bitangent;
	vec3 N = vertex.norm;
	mat3 TBN = mat3(T, B, N);

	outTBN = transpose(TBN);

	gl_Position =
		ubo.projView *
		vec4((chunk.position.xyz + vec3(x, y, z) +
			  vertex.pos),
			 1);
	outUv = vec3(vertex.uv, face.textureId);

	outNorm = vertex.norm;
	outPos = chunk.position.xyz + vec3(x, y, z) +
			 vertex.pos;
}
