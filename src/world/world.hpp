#pragma once
#include "chunk_data.hpp"
#include <memory>
#include <unordered_map>
namespace vblck
{
namespace world
{
constexpr uint32_t WORLD_SIZE = 16;
constexpr uint32_t WORLD_HEIGHT = 16;

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
	BlockDatabase blockDatabase;
	std::array<std::array<std::array<Chunk, WORLD_SIZE>, WORLD_HEIGHT>, WORLD_SIZE> chunks;

	Player player;

	std::vector<ChunkGenerateCommand> chunkGenerateCommands;

	inline const BlockData* getBlock(int32_t x, int32_t y, int32_t z)
	{
		if(x < 0 || y < 0 || z < 0)
			return 0;
		if(x >= (int32_t)(WORLD_SIZE * CHUNK_SIZE) || z >= (int32_t)(WORLD_SIZE * CHUNK_SIZE) ||
		   y >= (int32_t)(WORLD_HEIGHT * CHUNK_SIZE))
			return 0;

		auto cx = x / CHUNK_SIZE;
		auto cy = y / CHUNK_SIZE;
		auto cz = z / CHUNK_SIZE;

		x %= CHUNK_SIZE;
		y %= CHUNK_SIZE;
		z %= CHUNK_SIZE;

		return blockDatabase.getBlockFromId(chunks[cz][cy][cx].blocks[z][y][x]);
	}

	void create();

	void update(float deltaTime);

	static World* get()
	{
		static std::unique_ptr<World> world = std::make_unique<World>();
		return world.get();
	}

private:
	void updatePlayer(float deltaTime);
	void generateWorld();
};
} // namespace world
} // namespace vblck