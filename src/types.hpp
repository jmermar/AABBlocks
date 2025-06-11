#pragma once
#include <cstdint>
#include <glm/glm.hpp>
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
} // namespace vblck