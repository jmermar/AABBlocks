#pragma once
#include "chunk_data.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>
namespace vblck
{
namespace world
{

struct Player
{
	glm::vec3 position;
	glm::vec3 forward;

	float moveSpeed;

	void init();

	void rotateY(float degrees);
	void rotateX(float degrees);
};

struct ChunkGenerateCommand
{
	std::vector<ChunkFaceData> data;
	glm::vec3 position;
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
	uint32_t worldSize{}, worldHeight{};
	BlockDatabase blockDatabase;
	std::vector<Chunk> chunks;

	Player player;

	std::vector<ChunkGenerateCommand> chunkGenerateCommands;

	std::atomic<bool> loaded;
	std::atomic<float> progress;

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

	void generateChunkMeshes();

	void create(uint32_t worldSize, uint32_t worldHeight);

	void update(float deltaTime);

	bool drawGui();

	static World* get()
	{
		static std::unique_ptr<World> world = std::make_unique<World>();
		return world.get();
	}

private:
	void updatePlayer(float deltaTime);
};
} // namespace world
} // namespace vblck