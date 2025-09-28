struct ChunkFace
{
	uint posAndFace;
	uint textureId;
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
	uint first[8];
	uint count[8];
	float scale;
};