#include "types.hpp"
#include "utils/files.hpp"
#include <array>
#include <glm/glm.hpp>
namespace vblck
{
namespace world
{
constexpr uint32_t CHUNK_SIZE = 32;
struct Chunk
{
	glm::vec3 position;
	std::array<std::array<std::array<uint32_t, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> blocks;

	std::vector<ChunkFaceData> generateChunkData();
};
} // namespace world
} // namespace vblck