#include "world_generator.hpp"
#include "FastNoiseLite.h"
#include "world.hpp"
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
void WorldGenerator::initBlockIds()
{
	auto* db = &World::get()->blockDatabase;
	blockIds.dirt = db->getBlockId("Dirt");
	blockIds.grass = db->getBlockId("Grass");
	blockIds.sand = db->getBlockId("Sand");
	blockIds.stone = db->getBlockId("Stone");
}

void WorldGenerator::generateSolids()
{
	auto* world = World::get();
	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	std::atomic<uint32_t> nChunks;
#pragma omp parallel for
	for(uint32_t cz = 0; cz < world->worldSize; cz++)
	{
		for(uint32_t cx = 0; cx < world->worldSize; cx++)
		{
			for(uint32_t cy = 0; cy < world->worldHeight; cy++)
			{

				auto* chunk = world->chunkAt(cx, cy, cz);

				for(uint32_t z = 0; z < CHUNK_SIZE; z++)
				{

					for(uint32_t x = 0; x < CHUNK_SIZE; x++)
					{
						float wx = cx * CHUNK_SIZE + x;
						float wz = cz * CHUNK_SIZE + z;
						float n = fractalNoise(noise, wx * 0.5f, wz * 0.5f, 3, 1.5f);
						uint32_t h = baseHeight + n * baseAmplitude;
						for(uint32_t y = 0; y < CHUNK_SIZE; y++)
						{
							auto wy = cy * CHUNK_SIZE + y;
							uint32_t block = 0;

							if(wy <= h)
							{
								if(wy == h)
								{
									if(h <= sandLevel)
										block = blockIds.sand;
									else
										block = blockIds.grass;
								}
								else if(h - wy <= 2)
								{
									if(h <= sandLevel)
										block = blockIds.sand;
									else
										block = blockIds.dirt;
								}
								else
								{
									block = blockIds.stone;
								}
							}

							chunk->blocks[z][y][x] = block;
						}
					}
				}
				nChunks++;
				world->progress =
					(nChunks / (float)(world->worldHeight * world->worldSize * world->worldSize)) *
					0.5f;
			}
		}
	}
}

void WorldGenerator::generateWorld()
{
	initBlockIds();
	generateSolids();
}
} // namespace world
} // namespace vblck