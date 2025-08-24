#pragma once
#include "chunk_data.hpp"
#include "world.hpp"
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

	std::atomic<float> progress;
	std::atomic<bool> finished;
	std::vector<ChunkGenerateCommand> chunksToGenerate;

	void initBlockIds();

	void generateSolids();

	void generateChunkData();

	void generateNewWorld();
	void loadWorld();
};
} // namespace world
} // namespace vblck