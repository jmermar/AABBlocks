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
	auto cx = x / CHUNK_SIZE;
	auto cy = y / CHUNK_SIZE;
	auto cz = z / CHUNK_SIZE;

	auto* chunk = chunkAt(cx, cy, cz);
	if(!chunk)
		return;

	x %= CHUNK_SIZE;
	y %= CHUNK_SIZE;
	z %= CHUNK_SIZE;

	chunk->blocks[z][y][x] = id;
	dirtyChunks[chunk->getID()] = chunk;

	if(x == 0)
	{
		auto* chunk = chunkAt(cx - 1, cy, cz);
		if(chunk)
		{
			dirtyChunks[chunk->getID()] = chunk;
		}
	}
	if(y == 0)
	{
		auto* chunk = chunkAt(cx, cy - 1, cz);
		if(chunk)
		{
			dirtyChunks[chunk->getID()] = chunk;
		}
	}
	if(z == 0)
	{
		auto* chunk = chunkAt(cx, cy, cz - 1);
		if(chunk)
		{
			dirtyChunks[chunk->getID()] = chunk;
		}
	}

	if(x == CHUNK_SIZE - 1)
	{
		auto* chunk = chunkAt(cx + 1, cy, cz);
		if(chunk)
		{
			dirtyChunks[chunk->getID()] = chunk;
		}
	}

	if(y == CHUNK_SIZE - 1)
	{
		auto* chunk = chunkAt(cx, cy + 1, cz);
		if(chunk)
		{
			dirtyChunks[chunk->getID()] = chunk;
		}
	}

	if(z == CHUNK_SIZE - 1)
	{
		auto* chunk = chunkAt(cx, cy, cz + 1);
		if(chunk)
		{
			dirtyChunks[chunk->getID()] = chunk;
		}
	}
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
		if(cmd.data.size() > 0)
		{
			cmd.position = glm::vec3(c->cx, c->cy, c->cz) * (float)CHUNK_SIZE;
			cmd.chunk = c;
			chunkGenerateCommands.push_back(cmd);
		}
	}
	dirtyChunks.clear();
	for(auto& cmd : chunkGenerateCommands)
	{
		cmd.chunk->meshData =
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
	for(uint32_t cx = 0; cx < worldSize; cx++)
	{
		for(uint32_t cz = 0; cz < worldSize; cz++)
		{
			for(uint32_t cy = 0; cy < worldHeight; cy++)
			{
				auto* chunk = chunkAt(cx, cy, cz);
				chunk->cx = cx;
				chunk->cy = cy;
				chunk->cz = cz;
			}
		}
	}
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