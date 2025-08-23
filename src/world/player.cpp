#include "player.hpp"
#include "world/world.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

#include "input.hpp"
namespace vblck
{
namespace world
{
void Player::init()
{
	body.position = glm::vec3(1, 0, 1) * (float)(World::get()->worldSize * 0.5f * CHUNK_SIZE);
	body.size = glm::vec3(0.8f, 1.8f, 0.8f);
	body.position.y = 50;
	forward = glm::vec3(0, 0, 1);
}
void Player::rotateY(float degrees)
{
	auto angle = glm::radians(degrees / 2.f);
	glm::quat rotation(glm::cos(angle), glm::vec3(0, 1, 0) * glm::sin(angle));
	glm::quat rotationC = glm::conjugate(rotation);

	forward = rotation * forward * rotationC;
}
void Player::rotateX(float degrees)
{
	auto angle = glm::radians(degrees / 2.f);
	glm::quat rotation(glm::cos(angle),
					   glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))) * glm::sin(angle));
	glm::quat rotationC = glm::conjugate(rotation);

	forward = rotation * forward * rotationC;
}
void Player::move(glm::vec3 delta)
{
	grounded = false;
	if(delta.y < 0)
	{
		auto res = collisions::moveY(body, delta.y);
		if(res > delta.y + 0.0001)
		{
			grounded = true;
			velocity.y = 0;
		}
		body.position.y += res;
	}
	else
	{
		body.position.y += collisions::moveY(body, delta.y);
	}

	body.position.x += collisions::moveX(body, delta.x);
	body.position.z += collisions::moveZ(body, delta.z);
}
void Player::update(float deltaTime)
{

	rotateY(InputData::getAxis().x);
	rotateX(-InputData::getAxis().y);

	auto forward = this->forward;
	forward.y = 0;
	forward = glm::normalize(forward);

	auto right = glm::cross(glm::vec3(0, 1, 0), forward);
	glm::vec3 moveInput = glm::vec3(0);

	if(InputData::isDown(INPUT_MOVE_FORWARD))
	{
		moveInput += forward;
	}
	if(InputData::isDown(INPUT_MOVE_BACKWARD))
	{
		moveInput -= forward;
	}
	if(InputData::isDown(INPUT_MOVE_LEFT))
	{
		moveInput -= right;
	}
	if(InputData::isDown(INPUT_MOVE_RIGHT))
	{
		moveInput += right;
	}

	if(InputData::isDown(INPUT_JUMP) && grounded)
	{
		doJump = true;
	}

	if(glm::length(moveInput) > 0.2)
	{
		this->moveInput = moveInput;
	}
}

void Player::fixedUpdate()
{
	if(doJump)
	{
		doJump = false;
		velocity.y = jump;
	}

	auto world = World::get();
	velocity.x = moveInput.x * moveSpeed;
	velocity.z = moveInput.z * moveSpeed;
	velocity.y = std::max(-world->physicsData.maxFallSpeed,
						  velocity.y + world->physicsData.gravity * fixedDelta);
	move(velocity * fixedDelta);
	moveInput = glm::vec3(0);
}
} // namespace world
} // namespace vblck