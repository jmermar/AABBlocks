#include "world.hpp"
namespace vblck
{
namespace world
{
void World::generateWorld()
{
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
					for(uint32_t y = 0; y < CHUNK_SIZE; y++)
					{
						for(uint32_t x = 0; x < CHUNK_SIZE; x++)
						{
							auto h = cy * CHUNK_SIZE + y;
							if(h < (10 + ((x + z) % 8)))
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
			}
		}
	}
}
} // namespace world
} // namespace vblck