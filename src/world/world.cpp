#include "world.hpp"
#include "FastNoiseLite.h"
#include "input.hpp"
#include "rendering/renderer.hpp"
#include "world_generator.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <imgui.h>
#include <omp.h>
namespace vblck
{
namespace world
{

constexpr uint32_t SAND_LEVEL = 30;

void generateWorldThread()
{
	auto* world = World::get();
	world->chunks.resize(world->worldSize * world->worldSize * world->worldHeight);

	WorldGenerator generator;
	generator.baseAmplitude = 20;
	generator.baseHeight = 16;
	generator.sandLevel = 25;
	generator.world_height = world->worldHeight;
	generator.world_size = world->worldSize;
	generator.generateWorld();
	std::atomic<uint32_t> nChunks = 0;

	int numThreads = omp_get_max_threads();
	std::vector<std::vector<ChunkGenerateCommand>> localVectors(numThreads);

#pragma omp parallel for
	for(uint32_t cz = 0; cz < world->worldSize; cz++)
	{
		for(uint32_t cx = 0; cx < world->worldSize; cx++)
		{
			int tid = omp_get_thread_num();
			std::vector<ChunkGenerateCommand>& local = localVectors[tid];
			for(uint32_t cy = 0; cy < world->worldHeight; cy++)
			{

				auto* chunk = world->chunkAt(cx, cy, cz);
				ChunkGenerateCommand genCmd{};
				genCmd.data = chunk->generateChunkData();
				if(genCmd.data.size() > 0)
				{
					genCmd.position = glm::vec3(cx, cy, cz) * (float)CHUNK_SIZE;
					local.push_back(genCmd);
				}
				nChunks++;
				world->progress =
					(nChunks / (float)(world->worldHeight * world->worldSize * world->worldSize)) *
						0.5f +
					0.5f;
			}
		}
	}
	for(auto& local : localVectors)
	{
		world->chunkGenerateCommands.insert(
			world->chunkGenerateCommands.end(), local.begin(), local.end());
	}

	world->loaded = true;
}

void World::generateChunkMeshes()
{
	auto* renderer = render::Renderer::get();
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

	player.init();
	loaded = false;
	progress = 0;

	std::thread loadThread(generateWorldThread);
	loadThread.detach();
}
void World::update(float deltaTime)
{
	if(!loaded)
	{
		return;
	}
	updatePlayer(deltaTime);
}
bool World::drawGui()
{
	if(!loaded)
	{
		ImGui::Text("Loading world");
		ImGui::Text("Progress: %.2f", progress * 100.f);
		return false;
	}
	generateChunkMeshes();
	if(InputData::isPressed(INPUT_MENU))
	{
		InputData::setCaptureMosue(!InputData::getCaptureMosue());
	}

	if(!InputData::getCaptureMosue())
	{
		if(ImGui::Button("Return to title"))
		{
			return true;
		}
	}

	return false;
}
void World::updatePlayer(float deltaTime)
{
	auto right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), player.forward));
	if(InputData::isDown(INPUT_MOVE_FORWARD))
	{
		player.position += player.forward * deltaTime * player.moveSpeed;
	}
	if(InputData::isDown(INPUT_MOVE_BACKWARD))
	{
		player.position -= player.forward * deltaTime * player.moveSpeed;
	}
	if(InputData::isDown(INPUT_MOVE_LEFT))
	{
		player.position -= right * deltaTime * player.moveSpeed;
	}
	if(InputData::isDown(INPUT_MOVE_RIGHT))
	{
		player.position += right * deltaTime * player.moveSpeed;
	}

	player.rotateY(InputData::getAxis().x);
	player.rotateX(-InputData::getAxis().y);
}

void Player::init()
{
	position = glm::vec3(1, 0, 1) * (float)(World::get()->worldSize * 0.5f * CHUNK_SIZE);
	position.y = 50;
	forward = glm::vec3(0, 0, 1);
	moveSpeed = 40;
}
void Player::rotateY(float degrees)
{
	auto angle = glm::radians(degrees / 2.f);
	glm::quat rotation(glm::cos(angle), glm::vec3(0, 1, 0) * glm::sin(angle));
	glm::quat rotationC = glm::conjugate(rotation);

	forward = rotation * forward * rotationC;
}
void Player::rotateX(float degrees)
{
	auto angle = glm::radians(degrees / 2.f);
	glm::quat rotation(glm::cos(angle),
					   glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))) * glm::sin(angle));
	glm::quat rotationC = glm::conjugate(rotation);

	forward = rotation * forward * rotationC;
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