#pragma once
#include "chunk_data.hpp"
#include <memory>
#include <unordered_map>
namespace vblck
{
namespace world
{
struct WorldGenerator
{
	uint32_t world_size;
	uint32_t world_height;

	uint32_t baseHeight;
	uint32_t sandLevel;
	uint32_t baseAmplitude;

	struct
	{
		uint32_t sand;
		uint32_t grass;
		uint32_t stone;
		uint32_t dirt;
	} blockIds;

	void initBlockIds();

	void generateSolids();

	void generateWorld();
};
} // namespace world
} // namespace vblck