#version 450
#extension GL_EXT_buffer_reference : require

layout (location=0) out vec3 outUv;

struct ChunkFace {
	uint posAndFace;
	uint textureId;
};

struct Vertex {
	vec3 pos;
	vec2 uv;
};

layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer FacesAddr {
	ChunkFace faces[];
};

layout(set = 0, binding = 0) readonly uniform CameraData {
    mat4 proj;
    mat4 view;
    mat4 projView;
} ubo;

layout(std430, set=1, binding = 1) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform PushConstants {
    FacesAddr faces;
	vec3 position;
} pc;

void main() 
{
	uint idx = gl_VertexIndex;
	uint fineIdx = idx % 6;
    // Interpretar el address como un puntero a un array de VertexBuffer
    ChunkFace face = pc.faces.faces[idx / 6];
	uint faceId = face.posAndFace >> 24;
	float x = face.posAndFace & 0xff;
	float y = (face.posAndFace>>8) & 0xff;
	float z = (face.posAndFace>>16) & 0xff;
	Vertex vertex = vertices[faceId * 6 + fineIdx];

	gl_Position = ubo.projView * vec4((pc.position + vec3(x, y, z) + vertex.pos), 1);
	outUv = vec3(vertex.uv, face.textureId);
}
