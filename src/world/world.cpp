#include "world.hpp"
#include "FastNoiseLite.h"
#include "input.hpp"
#include "rendering/renderer.hpp"
#include "world_generator.hpp"
#include <imgui.h>
#include <omp.h>
namespace vblck
{
namespace world
{

constexpr uint32_t SAND_LEVEL = 30;

void World::setBlock(int32_t x, int32_t y, int32_t z, uint32_t id)
{
	auto* chunk = chunkAt(x / CHUNK_SIZE, y / CHUNK_SIZE, z / CHUNK_SIZE);
	if(!chunk)
		return;

	x %= CHUNK_SIZE;
	y %= CHUNK_SIZE;
	z %= CHUNK_SIZE;

	chunk->blocks[z][y][x] = id;
	dirtyChunks[chunk->getID()] = chunk;
}

void World::generateChunkMeshes()
{
	auto* renderer = render::Renderer::get();
	for(auto [id, c] : dirtyChunks)
	{
		ChunkGenerateCommand cmd;
		cmd.data = c->generateChunkData();
		if(c->meshData)
		{
			renderer->worldRenderer.chunkRenderer.deleteChunk(c->meshData);
		}
		cmd.position = glm::vec3(c->cx, c->cy, c->cz) * (float)CHUNK_SIZE;
		chunkGenerateCommands.push_back(cmd);
	}
	dirtyChunks.clear();
	for(auto& cmd : chunkGenerateCommands)
	{
		renderer->worldRenderer.chunkRenderer.loadChunk(cmd.position, cmd.data);
	}
	if(chunkGenerateCommands.size() > 0)
	{
		renderer->worldRenderer.chunkRenderer.regenerateChunks();
	}
	chunkGenerateCommands.clear();
}

void World::create(uint32_t worldSize, uint32_t worldHeight)
{
	blockDatabase.loadDatabase("res/BlockData.csv");
	this->worldHeight = worldHeight;
	this->worldSize = worldSize;
	chunks.resize(worldSize * worldSize * worldHeight);
}
void World::update(float deltaTime)
{
	generateChunkMeshes();
}

void BlockDatabase::loadDatabase(const std::string& file)
{
	blocks = loadBlockData(file);
	for(size_t i = 0; i < blocks.size(); i++)
	{
		maps[blocks[i].name] = i + 1;
	}
}
} // namespace world
} // namespace vblck