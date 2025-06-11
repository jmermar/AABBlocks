#include "chunk_data.hpp"

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
				bool isChunk = blocks[z][y][x] != 0;
				ChunkFaceData face;
				face.position = glm::vec3(x, y, z);
				if(isChunk && ((x == 0) || !blocks[z][y][x - 1]))
				{
					face.face = 3;
					face.textureId = 49;
					ret.push_back(face);
				}

				if(isChunk && ((x == CHUNK_SIZE - 1) || !blocks[z][y][x + 1]))
				{
					face.face = 2;
					face.textureId = 49;
					ret.push_back(face);
				}

				if(isChunk && ((z == 0) || !blocks[z - 1][y][x]))
				{
					face.face = 1;
					face.textureId = 49;
					ret.push_back(face);
				}

				if(isChunk && ((z == CHUNK_SIZE - 1) || !blocks[z + 1][y][x]))
				{
					face.face = 0;
					face.textureId = 49;
					ret.push_back(face);
				}

				if(isChunk && ((y == 0) || !blocks[z][y - 1][x]))
				{
					face.face = 5;
					face.textureId = 0;
					ret.push_back(face);
				}

				if(isChunk && ((y == CHUNK_SIZE - 1) || !blocks[z][y + 1][x]))
				{
					face.face = 4;
					face.textureId = 48;
					ret.push_back(face);
				}
			}
		}
	}
	return ret;
}
} // namespace world
} // namespace vblck
