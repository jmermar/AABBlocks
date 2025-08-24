#include "collisions.hpp"
#include "world/world.hpp"
namespace vblck
{
namespace collisions
{

inline int discretizeFloat(float a)
{
	if(std::abs(a) < 0.001)
		return a;
	if(a > 0)
	{
		int dis = a;
		if(a / dis > 0.001)
		{
			return a++;
		}
		return a;
	}
	return -discretizeFloat(-a);
}

AABBOverlapResult overlaps(const AABB& aabb, const AABB* ignore)
{
	auto* world = world::World::get();
	int minx = discretizeFloat(aabb.min().x);
	int miny = discretizeFloat(aabb.min().y);
	int minz = discretizeFloat(aabb.min().z);

	int maxx = discretizeFloat(aabb.max().x);
	int maxy = discretizeFloat(aabb.max().y);
	int maxz = discretizeFloat(aabb.max().z);

	AABBOverlapResult overlapResult;
	overlapResult.max = aabb.max();
	overlapResult.min = aabb.min();
	overlapResult.colision = false;
	AABB box;
	box.size = glm::vec3(1);
	for(int z = minz; z <= maxz; z++)
	{
		for(int x = minx; x <= maxx; x++)
		{
			for(int y = miny; y <= maxy; y++)
			{
				auto* block = world->getBlock(x, y, z);
				box.position = glm::vec3(x, y, z);
				if(block && block->solid && !(ignore && ignore->intersects(box)))
				{
					overlapResult.colision = true;
					overlapResult.min.x = std::min(overlapResult.min.x, (float)x);
					overlapResult.min.y = std::min(overlapResult.min.y, (float)y);
					overlapResult.min.z = std::min(overlapResult.min.z, (float)z);

					overlapResult.max.x = std::max(overlapResult.max.x, (float)x + 1);
					overlapResult.max.y = std::max(overlapResult.max.y, (float)y + 1);
					overlapResult.max.z = std::max(overlapResult.max.z, (float)z + 1);
				}
			}
		}
	}

	return overlapResult;
}

float moveX(const AABB& aabb, float distance, const AABB* ignore)
{
	if(std::abs(distance) < 0.001)
		return 0;
	auto dest = aabb;
	dest.position.x += distance;

	auto ovResult = overlaps(dest, ignore);
	if(!ovResult.colision)
	{
		return distance;
	}

	if(distance > 0)
	{
		distance = ovResult.min.x - aabb.size.x - aabb.position.x;
		if(distance > 0)
			return distance;
		return 0;
	}

	if(distance < 0)
	{
		distance = ovResult.max.x - aabb.position.x;
		if(distance < 0)
			return distance;
		return 0;
	}
	return 0;
}
float moveY(const AABB& aabb, float distance, const AABB* ignore)
{
	if(std::abs(distance) < 0.001)
		return 0;
	auto dest = aabb;
	dest.position.y += distance;

	auto ovResult = overlaps(dest, ignore);
	if(!ovResult.colision)
	{
		return distance;
	}

	if(distance > 0)
	{
		distance = ovResult.min.y - aabb.size.y - aabb.position.y;
		if(distance > 0)
			return distance;
		return 0;
	}

	if(distance < 0)
	{
		distance = ovResult.max.y - aabb.position.y;
		if(distance < 0)
			return distance;
		return 0;
	}
	return 0;
}

float moveZ(const AABB& aabb, float distance, const AABB* ignore)
{
	if(std::abs(distance) < 0.001)
		return 0;
	auto dest = aabb;
	dest.position.z += distance;

	auto ovResult = overlaps(dest, ignore);
	if(!ovResult.colision)
	{
		return distance;
	}

	if(distance > 0)
	{
		distance = ovResult.min.z - aabb.size.z - aabb.position.z;
		if(distance > 0)
			return distance;
		return 0;
	}

	if(distance < 0)
	{
		distance = ovResult.max.z - aabb.position.z;
		if(distance < 0)
			return distance;
		return 0;
	}
	return 0;
}
} // namespace collisions
} // namespace vblck