#pragma once
#include "chunk_data.hpp"
#include "collisions/collisions.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>
namespace vblck
{
namespace world
{
struct Player
{
	collisions::AABB body;
	glm::vec3 forward;
	glm::vec3 velocity;

	float moveSpeed = 4;
	float jump = 8;
	float grounded = false;

	bool doJump = false;

	glm::vec3 moveInput;

	void init();

	void rotateY(float degrees);
	void rotateX(float degrees);

	void move(glm::vec3 delta);

	void update(float deltaTime);
	void fixedUpdate();
};
} // namespace world
} // namespace vblck