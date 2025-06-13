#include "chunk_data.hpp"
#include "world.hpp"
namespace vblck
{
namespace world
{
std::vector<ChunkFaceData> Chunk::generateChunkData()
{
	std::vector<ChunkFaceData> ret;
	for(uint32_t z = 0; z < CHUNK_SIZE; z++)
	{
		for(uint32_t y = 0; y < CHUNK_SIZE; y++)
		{
			for(uint32_t x = 0; x < CHUNK_SIZE; x++)
			{
				auto* block = getBlock(x, y, z);
				if(block && block->solid)
				{
					ChunkFaceData face;
					face.x = x;
					face.y = y;
					face.z = z;

					if(!isSolid(x - 1, y, z))
					{
						face.face = CHUNK_FACES_LEFT;
						face.textureId = block->faces[CHUNK_FACES_LEFT];
						ret.push_back(face);
					}

					if(!isSolid(x + 1, y, z))
					{
						face.face = CHUNK_FACES_RIGHT;
						face.textureId = block->faces[CHUNK_FACES_RIGHT];
						ret.push_back(face);
					}

					if(!isSolid(x, y, z - 1))
					{
						face.face = CHUNK_FACES_BACK;
						face.textureId = block->faces[CHUNK_FACES_BACK];
						ret.push_back(face);
					}

					if(!isSolid(x, y, z + 1))
					{
						face.face = CHUNK_FACES_FRONT;
						face.textureId = block->faces[CHUNK_FACES_FRONT];
						ret.push_back(face);
					}

					if(!isSolid(x, y + 1, z))
					{
						face.face = CHUNK_FACES_TOP;
						face.textureId = block->faces[CHUNK_FACES_TOP];
						ret.push_back(face);
					}

					if(!isSolid(x, y - 1, z))
					{
						face.face = CHUNK_FACES_BOTTOM;
						face.textureId = block->faces[CHUNK_FACES_BOTTOM];
						ret.push_back(face);
					}
				}
			}
		}
	}
	return ret;
}
const BlockData* Chunk::getBlock(int32_t x, int32_t y, int32_t z)
{
	auto* world = World::get();
	if(x < 0 || y < 0 || z < 0 || x >= (int32_t)CHUNK_SIZE || y >= (int32_t)CHUNK_SIZE ||
	   z >= (int32_t)CHUNK_SIZE)
	{
		return world->getBlock(cx * CHUNK_SIZE + x, cy * CHUNK_SIZE + y, cz * CHUNK_SIZE + z);
	}

	return world->blockDatabase.getBlockFromId(blocks[z][y][x]);
}
} // namespace world
} // namespace vblck
