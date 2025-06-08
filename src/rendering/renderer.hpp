#include "buffer_writter.hpp"
#include "command_buffer.hpp"
#include "staging_buffer.hpp"
#include "types.hpp"
#include "vk/deletion.hpp"
#include "vk/textures.hpp"
#include <functional>
#include <memory>
#include <vector>
namespace vblck
{
namespace render
{

constexpr unsigned int FRAME_OVERLAP = 2;

struct FrameData
{
	VkSemaphore swapchainSemaphore;
	VkFence renderFence;

	VkCommandPool commandPool;
	std::unique_ptr<CommandBuffer> mainCommandBuffer;

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
	std::vector<VkSemaphore> renderSemaphores{};
	VkFormat swapchainImageFormat{};

	VkExtent2D screenExtent{};

	vk::DeletionQueue mainDeletionQueue;
	vk::DeletionQueue frameDeletionQueue;

	FrameData frames[FRAME_OVERLAP];

	BufferWritter bufferWritter;

	vk::Texture2D backbuffer;

	void initVMA();
	void initCommands();
	void initSyncStructures();
	void initIMGUI();

	void destroySwapchain();
	void cleanup();

	void renderLogic(CommandBuffer* cmd);
	void renderImGUI(VkCommandBuffer cmd, VkImageView targetImageView);

	inline void deletePendingObjects()
	{
		for(size_t i = 0; i < FRAME_OVERLAP; i++)
		{
			frames[i].deletionQueue.deleteQueue(device, vma);
		}
		frameDeletionQueue.deleteQueue(device, vma);
	}

	static Renderer* renderInstance;

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
		renderInstance = this;
		frameNumber = 0;
		initVMA();
		recreateSwapchain(w, h);
		initCommands();
		initSyncStructures();
	}

	~Renderer()
	{
		cleanup();
	}

	static inline Renderer* get()
	{
		return renderInstance;
	}

	FrameData& getCurrentFrame()
	{
		return frames[frameNumber % FRAME_OVERLAP];
	};

	vk::Texture2D loadTexture2D(const char* path);

	void recreateSwapchain(int w, int h);

	void renderFrame();
};
} // namespace render
} // namespace vblck