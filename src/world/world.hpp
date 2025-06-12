#include "chunk_data.hpp"
#include <memory>
namespace vblck
{
namespace world
{
constexpr uint32_t WORLD_SIZE = 32;
constexpr uint32_t WORLD_HEIGHT = 16;

struct Player
{
	glm::vec3 position;
	glm::vec3 forward;

	float moveSpeed;

	void init();

	void rotateY(float degrees);
	void rotateX(float degrees);
};

struct ChunkGenerateCommand
{
	std::vector<ChunkFaceData> data;
	glm::vec3 position;
};

struct World
{
	std::array<std::array<std::array<Chunk, WORLD_SIZE>, WORLD_HEIGHT>, WORLD_SIZE> chunks;

	Player player;

	std::vector<ChunkGenerateCommand> chunkGenerateCommands;

	void generateWorld();

	void update(float deltaTime);

	static World* get()
	{
		static std::unique_ptr<World> world = std::make_unique<World>();
		return world.get();
	}

private:
	void updatePlayer(float deltaTime);
};
} // namespace world
} // namespace vblck