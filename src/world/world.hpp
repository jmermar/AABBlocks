#pragma once
#include "chunk_data.hpp"
#include "collisions/collisions.hpp"
#include "player.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>
#include <unordered_set>
namespace vblck
{
namespace world
{

struct ChunkGenerateCommand
{
	std::vector<ChunkFaceData> data;
	glm::vec3 position;
	Chunk* chunk;
};

struct BlockDatabase
{
	std::unordered_map<std::string, uint32_t> maps;
	std::vector<BlockData> blocks;

	inline const BlockData* getBlockFromId(size_t id)
	{
		assert(id <= blocks.size());
		if(id == 0)
			return 0;
		return &blocks[id - 1];
	}

	inline const BlockData* getBlockFromName(const std::string& name)
	{
		assert(maps.contains(name));
		return &blocks[maps[name]];
	}

	inline uint32_t getBlockId(const std::string& name)
	{
		assert(maps.contains(name));
		return maps[name];
	}

	void loadDatabase(const std::string& file);
};

struct World
{
	struct
	{
		float gravity = -19.6;
		float maxFallSpeed = 16;
	} physicsData;

	uint32_t worldSize{}, worldHeight{};
	BlockDatabase blockDatabase;
	std::vector<Chunk> chunks;

	std::unordered_map<uint64_t, Chunk*> dirtyChunks;
	std::vector<ChunkGenerateCommand> chunkGenerateCommands;

	inline Chunk* chunkAt(int32_t cx, int32_t cy, int32_t cz)
	{
		if(cx < 0 || cy < 0 || cz < 0)
			return 0;
		if(cx >= (int32_t)(worldSize) || cz >= (int32_t)(worldSize) || cy >= (int32_t)(worldHeight))
			return 0;

		return &chunks[cz * worldSize * worldHeight + cy * worldSize + cx];
	}

	inline const BlockData* getBlock(int32_t x, int32_t y, int32_t z)
	{
		auto* chunk = chunkAt(x / CHUNK_SIZE, y / CHUNK_SIZE, z / CHUNK_SIZE);
		if(!chunk)
			return 0;

		x %= CHUNK_SIZE;
		y %= CHUNK_SIZE;
		z %= CHUNK_SIZE;

		return blockDatabase.getBlockFromId(chunk->blocks[z][y][x]);
	}

	void setBlock(int32_t x, int32_t y, int32_t z, uint32_t id);

	void generateChunkMeshes();

	void create(uint32_t worldSize, uint32_t worldHeight);
	void destroy();

	void update(float deltaTime);

	static World* get()
	{
		static std::unique_ptr<World> world = std::make_unique<World>();
		return world.get();
	}

	void clear()
	{
		worldSize = 0;
		worldHeight = 0;
		chunks.clear();
		chunks.shrink_to_fit();
	}

private:
	void updatePlayer(float deltaTime);
};
} // namespace world
} // namespace vblck