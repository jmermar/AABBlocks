#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <vector>
namespace vblck
{

constexpr uint32_t fixedTicks = 50;
constexpr float fixedDelta = 1.f / fixedTicks;

enum class Result
{
	SUCESS,
	ERROR
};

struct ChunkFaceData
{
	uint8_t x, y, z;
	uint8_t face;
	uint32_t textureId;
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