#include "chunk_data.hpp"
#include "world.hpp"
namespace vblck
{
namespace world
{

u_int16_t modalId(Chunk* chunk,
				  uint16_t x,
				  uint16_t y,
				  uint16_t z,
				  size_t lod)
{
	std::unordered_map<uint16_t, uint32_t> counts;
	for(uint32_t oz = 0; oz < 2; oz++)
	{
		for(uint32_t oy = 0; oy < 2; oy++)
		{
			for(uint32_t ox = 0; ox < 2; ox++)
			{
				auto b =
					chunk->getBlockLOD(x * 2 + ox,
									   y * 2 + oy,
									   z * 2 + oz,
									   lod - 1);
				if(b)
				{
					if(!counts.contains(b))
					{
						counts[b] = 0;
					}
					counts[b]++;
				}
			}
		}
	}
	uint16_t block = 0;
	uint32_t maxCount = 0;
	for(auto [b, c] : counts)
	{
		if(c > maxCount)
		{
			block = b;
			maxCount = c;
		}
	}
	return block;
}

std::vector<ChunkFaceData>
Chunk::generateChunkData()
{
	std::vector<ChunkFaceData> ret;
	for(uint32_t z = 0; z < CHUNK_SIZE; z++)
	{
		for(uint32_t y = 0; y < CHUNK_SIZE; y++)
		{
			for(uint32_t x = 0; x < CHUNK_SIZE;
				x++)
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
						face.face =
							CHUNK_FACES_LEFT;
						face.textureId =
							block->faces
								[CHUNK_FACES_LEFT];
						ret.push_back(face);
					}

					if(!isSolid(x + 1, y, z))
					{
						face.face =
							CHUNK_FACES_RIGHT;
						face.textureId =
							block->faces
								[CHUNK_FACES_RIGHT];
						ret.push_back(face);
					}

					if(!isSolid(x, y, z - 1))
					{
						face.face =
							CHUNK_FACES_BACK;
						face.textureId =
							block->faces
								[CHUNK_FACES_BACK];
						ret.push_back(face);
					}

					if(!isSolid(x, y, z + 1))
					{
						face.face =
							CHUNK_FACES_FRONT;
						face.textureId =
							block->faces
								[CHUNK_FACES_FRONT];
						ret.push_back(face);
					}

					if(!isSolid(x, y + 1, z))
					{
						face.face =
							CHUNK_FACES_TOP;
						face.textureId =
							block->faces
								[CHUNK_FACES_TOP];
						ret.push_back(face);
					}

					if(!isSolid(x, y - 1, z))
					{
						face.face =
							CHUNK_FACES_BOTTOM;
						face.textureId =
							block->faces
								[CHUNK_FACES_BOTTOM];
						ret.push_back(face);
					}
				}
			}
		}
	}
	return ret;
}
const BlockData*
Chunk::getBlock(int32_t x, int32_t y, int32_t z)
{
	auto* world = World::get();
	if(x < 0 || y < 0 || z < 0 ||
	   x >= (int32_t)CHUNK_SIZE ||
	   y >= (int32_t)CHUNK_SIZE ||
	   z >= (int32_t)CHUNK_SIZE)
	{
		return world->getBlock(
			cx * CHUNK_SIZE + x,
			cy * CHUNK_SIZE + y,
			cz * CHUNK_SIZE + z);
	}

	return world->blockDatabase.getBlockFromId(
		blocks[z][y][x]);
}
void Chunk::regenerateLODs()
{
	for(size_t lod = 1; lod <= 3; lod++)
	{
		for(uint32_t z = 0; z < CHUNK_SIZE >> lod;
			z++)
		{
			for(uint32_t y = 0;
				y < CHUNK_SIZE >> lod;
				y++)
			{
				for(uint32_t x = 0;
					x < CHUNK_SIZE >> lod;
					x++)
				{
					uint16_t block = modalId(
						this, x, y, z, lod);

					setBlockLOD(
						x, y, z, block, lod);
				}
			}
		}
	}
}
uint64_t Chunk::getID()
{
	auto* world = World::get();
	return cx * world->worldSize *
			   world->worldSize +
		   cz * world->worldSize + cy;
}
} // namespace world
} // namespace vblck
