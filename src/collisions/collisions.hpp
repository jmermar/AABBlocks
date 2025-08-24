#pragma once
#include <glm/glm.hpp>
namespace vblck
{

struct BlockData;
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

	inline bool intersects(const AABB& other) const
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

		return true;
	}
	inline bool pointIn(glm::vec3 p) const
	{
		if(p.x < min().x)
			return false;
		if(p.y < min().y)
			return false;
		if(p.z < min().z)
			return false;

		if(p.x > max().x)
			return false;
		if(p.y > max().y)
			return false;
		if(p.z > max().z)
			return false;

		return true;
	}
};

struct AABBOverlapResult
{
	glm::vec3 min, max;
	bool colision;
};

struct RaycastResult
{
	glm::vec3 hitpoint;
	const BlockData* block;
	glm::vec3 norm;
};

AABBOverlapResult overlaps(const AABB& aabb, const AABB* ignore = 0);

float moveX(const AABB& aabb, float distance, const AABB* ignore = 0);
float moveY(const AABB& aabb, float distance, const AABB* ignore = 0);
float moveZ(const AABB& aabb, float distance, const AABB* ignore = 0);

RaycastResult raycast(glm::vec3 pos, glm::vec3 dir, float distance);
} // namespace collisions

} // namespace vblck