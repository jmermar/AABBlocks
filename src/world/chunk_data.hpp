#pragma once
#include "types.hpp"
#include "utils/files.hpp"
#include <array>
#include <glm/glm.hpp>
namespace vblck
{
namespace render
{
struct ChunkData;
}
namespace world
{
constexpr uint32_t CHUNK_SIZE = 32;
struct Chunk
{
	template <size_t S>
	using ChunkData = std::array<
		std::array<std::array<uint16_t, S>, S>,
		S>;

	uint32_t cx, cy, cz;
	ChunkData<CHUNK_SIZE> blocks;
	ChunkData<CHUNK_SIZE / 2> lod1;
	ChunkData<CHUNK_SIZE / 4> lod2;
	ChunkData<CHUNK_SIZE / 8> lod3;
	render::ChunkData* meshData{};

	std::vector<ChunkFaceData>
	generateChunkData();
	const BlockData*
	getBlock(int32_t x, int32_t y, int32_t z);
	void regenerateLODs();
	inline bool
	isSolid(int32_t x, int32_t y, int32_t z)
	{
		auto* bd = getBlock(x, y, z);
		return bd && bd->solid;
	}

	inline uint16_t getBlockLOD(uint16_t x,
								uint16_t y,
								uint16_t z,
								size_t lod = 0)
	{
		if(x >= CHUNK_SIZE >> lod ||
		   y >= CHUNK_SIZE >> lod ||
		   z >= CHUNK_SIZE >> lod)
			return 0;
		if(lod == 0)
			return blocks[z][y][x];
		if(lod == 1)
			return lod1[z][y][x];
		if(lod == 2)
			return lod2[z][y][x];
		if(lod == 3)
			return lod3[z][y][x];
		return 0;
	}

	inline void setBlockLOD(uint16_t x,
							uint16_t y,
							uint16_t z,
							uint16_t id,
							size_t lod = 0)
	{
		if(x >= CHUNK_SIZE >> lod ||
		   y >= CHUNK_SIZE >> lod ||
		   z >= CHUNK_SIZE >> lod)
			return;
		if(lod == 0)
			blocks[z][y][x] = id;
		else if(lod == 1)
			lod1[z][y][x] = id;
		else if(lod == 2)
			lod2[z][y][x] = id;
		else if(lod == 3)
			lod3[z][y][x] = id;
	}

	bool isEmpty()
	{
		auto* data = (uint16_t*)blocks.data();
		for(size_t i = 0;
			i <
			CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
			i++)
		{
			if(data[i])
				return false;
		}
		return true;
	}

	uint64_t getID();
};
} // namespace world
} // namespace vblck