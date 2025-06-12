#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <vector>
namespace vblck
{
enum class Result
{
	SUCESS,
	ERROR
};

struct ChunkFaceData
{
	glm::vec3 position;
	uint32_t face;
	uint32_t textureId;
	uint32_t pad[3];
};

enum ChunkFaces
{
	CHUNK_FACES_FRONT,
	CHUNK_FACES_BACK,
	CHUNK_FACES_RIGHT,
	CHUNK_FACES_LEFT,
	CHUNK_FACES_TOP,
	CHUNK_FACES_BOTTOM
};

struct BlockData
{
	std::string name;
	bool solid;
	uint32_t faces[6];
};
} // namespace vblck