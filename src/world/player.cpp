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
	body.position.y += collisions::moveY(body, delta.y);
	body.position.x += collisions::moveX(body, delta.x);
	body.position.z += collisions::moveZ(body, delta.z);
}
void Player::update(float deltaTime)
{
	auto world = World::get();
	velocity.x = 0;
	velocity.z = 0;

	velocity.y = std::max(-world->physicsData.maxFallSpeed,
						  velocity.y + world->physicsData.gravity * deltaTime);

	rotateY(InputData::getAxis().x);
	rotateX(-InputData::getAxis().y);

	auto forward = this->forward;
	forward.y = 0;
	forward = glm::normalize(forward);

	auto right = glm::cross(glm::vec3(0, 1, 0), forward);

	if(InputData::isDown(INPUT_MOVE_FORWARD))
	{
		velocity += forward * moveSpeed;
	}
	if(InputData::isDown(INPUT_MOVE_BACKWARD))
	{
		velocity -= forward * moveSpeed;
	}
	if(InputData::isDown(INPUT_MOVE_LEFT))
	{
		velocity -= right * moveSpeed;
	}
	if(InputData::isDown(INPUT_MOVE_RIGHT))
	{
		velocity += right * moveSpeed;
	}

	if(InputData::isPressed(INPUT_JUMP))
	{
		velocity.y = jump;
	}

	move(velocity * deltaTime);
}
} // namespace world
} // namespace vblck