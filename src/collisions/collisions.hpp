#pragma once
#include <glm/glm.hpp>

namespace vblck
{
namespace collisions
{
struct AABB
{
	glm::vec3 position;
	glm::vec3 size;

	inline glm::vec3 min() const
	{
		return position;
	}

	inline glm::vec3 max() const
	{
		return position + size;
	}

	inline bool intersects(const AABB& other)
	{
		if(min().x > other.max().x)
			return false;

		if(max().x < other.min().x)
			return false;

		if(min().y > other.max().y)
			return false;

		if(max().y < other.min().y)
			return false;

		if(min().z > other.max().z)
			return false;

		if(max().z < other.min().z)
			return false;
	}
};

struct AABBOverlapResult
{
	glm::vec3 min, max;
	bool colision;
};

AABBOverlapResult overlaps(const AABB& aabb);

float moveX(const AABB& aabb, float distance);
float moveY(const AABB& aabb, float distance);
float moveZ(const AABB& aabb, float distance);
} // namespace collisions

} // namespace vblck