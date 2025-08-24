#include "collisions.hpp"
#include "world/world.hpp"
namespace vblck
{
namespace collisions
{
RaycastResult raycast(glm::vec3 pos, glm::vec3 dir, float distance)
{
	constexpr float delta = 0.05f;
	auto world = world::World::get();
	dir = glm::normalize(dir);
	RaycastResult result;
	int prevx = (int32_t)pos.x;
	int prevy = (int32_t)pos.y;
	int prevz = (int32_t)pos.z;
	for(float i = 0; i <= distance; i += delta)
	{
		int x = (int32_t)pos.x;
		int y = (int32_t)pos.y;
		int z = (int32_t)pos.z;
		auto block = world->getBlock(x, y, z);
		if(block && block->solid)
		{
			result.block = block;
			result.hitpoint = pos;
			result.norm =
				glm::normalize(glm::vec3((float)prevx - x, (float)prevy - y, (float)prevz - z));
			return result;
		}
		prevx = x;
		prevy = y;
		prevz = z;
		pos += dir * delta;
	}

	result.block = 0;
	return result;
}
} // namespace collisions
} // namespace vblck