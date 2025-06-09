#include "types.hpp"
#include <span>
namespace vblck
{
namespace vk
{
struct DescriptorLayoutBuilder
{

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	void addBinding(uint32_t binding, VkDescriptorType type);
	void clear();
	VkDescriptorSetLayout build(VkDevice device,
								VkShaderStageFlags shaderStages,
								void* pNext = nullptr,
								VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator
{

	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

	VkDescriptorPool pool;

	void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
	void clearDescriptors(VkDevice device);

	inline void freeDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet)
	{
		vkFreeDescriptorSets(device, pool, 1, &descriptorSet);
	}

	VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

} // namespace vk
} // namespace vblck