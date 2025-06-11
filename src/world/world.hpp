#include "chunk_data.hpp"
namespace vblck
{
namespace world
{
constexpr uint32_t WORLD_SIZE = 32;
constexpr uint32_t WORLD_HEIGHT = 16;
struct World
{
	std::array<std::array<std::array<Chunk, WORLD_SIZE>, WORLD_HEIGHT>, WORLD_SIZE> chunks;

	void generateWorld();
};
} // namespace world
} // namespace vblck