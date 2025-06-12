#pragma once
#include "types.hpp"
#include "utils/files.hpp"
#include <array>
#include <glm/glm.hpp>
namespace vblck
{
namespace world
{
constexpr uint32_t CHUNK_SIZE = 32;
struct Chunk
{
	glm::vec3 position;
	std::array<std::array<std::array<uint16_t, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> blocks;

	std::vector<ChunkFaceData> generateChunkData();
	const BlockData* getBlock(int32_t x, int32_t y, int32_t z);
	inline bool isSolid(int32_t x, int32_t y, int32_t z)
	{
		auto* bd = getBlock(x, y, z);
		return !(!bd || !bd->solid);
	}
};
} // namespace world
} // namespace vblck