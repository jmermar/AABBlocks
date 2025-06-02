#include "types.hpp"
#include "vk/deletion.hpp"
#include <functional>
#include <vector>
namespace vblck
{
namespace render
{

constexpr unsigned int FRAME_OVERLAP = 2;

struct FrameData
{
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	vk::DeletionQueue deletionQueue;
};

class Renderer
{
private:
	int frameNumber;
	VkInstance instance{};
	VkPhysicalDevice chosenGPU{};
	VkDevice device{};
	VkSurfaceKHR surface{};
	VkQueue graphicsQueue;
	VmaAllocator vma{};
	uint32_t graphicsQueueFamily;

	VkSwapchainKHR swapchain{};
	std::vector<VkImage> swapchainImages{};
	std::vector<VkImageView> swapchainImageViews{};
	VkFormat swapchainImageFormat;

	VkExtent2D screenExtent{};

	vk::DeletionQueue mainDeletionQueue;

	FrameData frames[FRAME_OVERLAP];

	FrameData& getCurrentFrame()
	{
		return frames[frameNumber % FRAME_OVERLAP];
	};

	void initCommands();
	void initSyncStructures();

	void destroySwapchain();

	void cleanup();

public:
	Renderer(VkInstance instance,
			 VkPhysicalDevice chosenGPU,
			 VkDevice device,
			 VkSurfaceKHR surface,
			 VkQueue graphicsQueue,
			 uint32_t graphicsQueueFamily,
			 int w,
			 int h)
		: instance(instance)
		, chosenGPU(chosenGPU)
		, device(device)
		, surface(surface)
		, graphicsQueue(graphicsQueue)
		, graphicsQueueFamily(graphicsQueueFamily)
	{
		frameNumber = 0;
		recreateSwapchain(w, h);
		initCommands();
		initSyncStructures();
	}

	~Renderer()
	{
		cleanup();
	}

	void recreateSwapchain(int w, int h);

	void renderFrame();
};
} // namespace render
} // namespace vblck