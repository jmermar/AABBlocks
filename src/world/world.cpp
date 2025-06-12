#include "world.hpp"
#include "FastNoiseLite.h"
#include "input.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
namespace vblck
{
namespace world
{

float fractalNoise(FastNoiseLite& noiseGen, float x, float z, size_t nOctaves, float decay)
{
	float noise = 0;
	float amp = 1;
	float freq = 1;
	float acc = 0;
	for(size_t i = 0; i < nOctaves; i++)
	{
		acc += amp;
		noise += noiseGen.GetNoise(x * freq, z * freq) * amp;
		freq *= 2;
		amp /= decay;
	}

	return noise / acc;
}
void World::generateWorld()
{
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	for(uint32_t cz = 0; cz < WORLD_SIZE; cz++)
	{
		for(uint32_t cy = 0; cy < WORLD_HEIGHT; cy++)
		{
			for(uint32_t cx = 0; cx < WORLD_SIZE; cx++)
			{
				auto& chunk = chunks[cz][cy][cx];
				chunk.position = glm::vec3(cx, cy, cz) * (float)CHUNK_SIZE;
				for(uint32_t z = 0; z < CHUNK_SIZE; z++)
				{
					for(uint32_t x = 0; x < CHUNK_SIZE; x++)
					{
						float wx = cx * CHUNK_SIZE + x;
						float wz = cz * CHUNK_SIZE + z;
						float n = fractalNoise(noise, wx * 0.5f, wz * 0.5f, 12, 1.8f);
						uint32_t h = 30 + n * 15;
						for(uint32_t y = 0; y < CHUNK_SIZE; y++)
						{

							if(cy * CHUNK_SIZE + y <= h)
							{
								chunk.blocks[z][y][x] = 1;
							}
							else
							{
								chunk.blocks[z][y][x] = 0;
							}
						}
					}
				}
				ChunkGenerateCommand genCmd{};
				genCmd.data = chunk.generateChunkData();
				if(genCmd.data.size() > 0)
				{
					genCmd.position = chunk.position;
					chunkGenerateCommands.push_back(genCmd);
				}
			}
		}
	}
	player.init();
}
void World::update(float deltaTime)
{
	if(InputData::isPressed(INPUT_MENU))
	{
		InputData::setCaptureMosue(!InputData::getCaptureMosue());
	}
	updatePlayer(deltaTime);
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
	position = glm::vec3(1, 0, 1) * (float)(WORLD_SIZE * 0.5f * CHUNK_SIZE);
	position.y = 50;
	forward = glm::vec3(0, 0, 1);
	moveSpeed = 8;
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
} // namespace world
} // namespace vblck